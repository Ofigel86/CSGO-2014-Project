# Packet Manager & Fake Angle - Reverse Engineering Report (CS:GO 2014-10-23)

## Цель
Сделать хороший пакет менеджер для фейковых углов (desync) под билд 2014-10-23 (VClient016, client.dll 12MB).

## Что отревёрсили

### 1. bSendPacket в CL_Move (engine.dll)
В оригинальном SDK 2007 (source):

```cpp
bool bSendPacket = true;
if (( !cl.m_NetChannel->IsLoopback() || host_limitlocal ) &&
    ( net_time < cl.m_flNextCmdTime || !cl.m_NetChannel->CanPacket() || !bFinalTick ))
    bSendPacket = false;

if (cl.IsActive()) {
  int nextcommandnr = cl.lastoutgoingcommand + cl.chokedcommands + 1;
  g_ClientDLL->CreateMove(nextcommandnr, ...);
  if (bSendPacket) CL_SendMove();
  else { cl.m_NetChannel->SetChoked(); cl.chokedcommands++; }
}
```

В CS:GO 2014 `bSendPacket` лежит на стеке CreateMove:
- В хуке CreateMove (index 21) берем ebp: `frame_ptr = ebp`
- `bSendPacket = *(bool*)(frame_ptr - 0x1)` (в 2022 патче стало ebp-0x34, в 2014 - ebp-0x1)
- Это уже было в проекте: `send_packet_ptr = *(PDWORD)(frame_ptr) - 0x1`

### 2. ClientState (engine.dll)
Сигнатура: `A1 ? ? ? ? 8B 80 ? ? ? ? C3` -> `mov eax, [ClientState]`

Структура для 2014-10-23 (из UC, проверено по строкам CBaseClientState):

```
0x9C: m_NetChannel (INetChannel*)
0x108: m_nSignonState (6 = FULL_CONNECTED)
0x110: m_flNextCmdTime
0x4D2C: m_nLastOutgoingCommand
0x4D30: m_nChokedCommands (было 0x4CB0 в старых билдах, в 2014 -> 0x4D30)
0x4D34: m_nLastCommandAck
0x4D38: m_nCommandAck
```

В INetChannel:
```
0x2C: m_nChokedPackets (альтернативный счетчик)
```

### 3. Packet Manager логика
- `can_choke(max=14)` - sv_maxusrcmdprocessticks = 16, безопасно 14
- `get_choked_commands()` из ClientState + 0x4D30
- `set_send_packet(bool)` пишет в `*bSendPacket`
- `force_send()` для сброса чока

### 4. Fake Angle (AntiAim) для 2014

#### Desync 58
- Max desync delta в 2014 = 58° (из CCSGOPlayerAnimState, когда скорость <0.1)
- При движении падает до 35°
- Реализация:
  ```
  if choked < 1:
    viewangles.yaw = original.yaw + 58 (fake)
    bSendPacket = false (чок)
  else:
    viewangles.yaw = original.yaw (real)
    bSendPacket = true (отправка)
  ```
- Сервер видит real, враги видят fake (из-за чока)

#### LBY Breaker
- LBY (Lower Body Yaw) обновляется каждые 1.1 сек стоя, 0.22 сек в движении
- Брейк: за 1 тик до обновления ставим yaw +180 и чокаем, LBY становится фейковым
- Реверс из server.dll: `CCSPlayer::UpdateLBY`

#### Другие моды
- Backwards: real = +180
- Sideways: real +90 / fake -90
- Jitter: переключает стороны каждый тик
- 180 static: классика

### 5. Интеграция в CreateMove
```
bool send_packet_state = *bSendPacket
g_antiaim->instance(cmd, send_packet_state) // меняет viewangles и send_packet_state
*g_packet_manager->set_send_packet(send_packet_state)
*bSendPacket = send_packet_state

// Fake lag отдельно
if choked < fakelag_ticks -> *bSendPacket = false
```

## Файлы
- `utilities/managers/packet_manager.hpp/cpp` - менеджер пакетов, реверс ClientState
- `hacks/antiaim.hpp/cpp` - фейк углы, desync 58, LBY breaker
- `hooks/functions/client.cpp` - интеграция, bSendPacket handling
- `config/config.hpp` - настройки: yaw_mode, pitch_mode, fakelag
- `gui/other/content_tabs.cpp` - GUI для антиаима

## Дальнейшее
- Добавить m_vecVelocity netvar (CBasePlayer, m_vecVelocity[0]) для точного max desync
- Добавить m_MoveType netvar для проверки noclip/ladder
- Реверс CCSGOPlayerAnimState для точного GetMaxDesyncDelta (сейчас 58 статично)
- Добавить визуализацию real/fake в ESP
- Fake lag на основе netchannel

## Проверка сборки
Собрано через zig: `python -m ziglang c++ -target x86-windows-gnu -shared -o csgosdk.dll 45 cpp`
Результат 1.8MB PE32 валидный.

## Ссылки
- UC: CL_Move reverse https://www.unknowncheats.me/forum/counterstrike-global-offensive/115717-reverse-cl_move.html
- UC: ClientState struct https://www.unknowncheats.me/forum/counterstrike-global-offensive/103220-counterstrike-global-offensive-reversal-structs-offsets-689.html
- UC: bSendPacket manipulation https://www.unknowncheats.me/forum/counterstrike-global-offensive/294986-manipulating-bsendpacket-touching-text-hooks.html
