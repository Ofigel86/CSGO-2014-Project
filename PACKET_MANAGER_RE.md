# Packet Manager & Fake Angle - Reverse Engineering Report (CS:GO 2014-10-23)

## Билд игры (из твоих DLL)
- **engine.dll**: Exe build: 13:22:52 Oct 15 2014, PE timestamp 1414021165 -> 2014-10-22 23:39:25
- **client.dll**: 12MB, PE timestamp 1414102115 -> 2014-10-23 22:08:35, VClient016
- **Версия**: 1.34.6.x, Oct 2014, именно тот билд что нужен (VClient016)
- **Важно**: В 2014 НЕТУ LBY! LBY (Lower Body Yaw) добавили в 2015-2016. Так что LBY breaker - бред для 2014.

## Что отревёрсили из твоих DLL

### 1. bSendPacket в CL_Move (engine.dll)
Реверс из engine.dll + SDK 2007:

```cpp
bool bSendPacket = true;
if (( !cl.m_NetChannel->IsLoopback() || host_limitlocal ) &&
    ( net_time < cl.m_flNextCmdTime || !CanPacket() || !bFinalTick ))
    bSendPacket = false;

int nextcommandnr = lastoutgoingcommand + chokedcommands + 1;
g_ClientDLL->CreateMove(nextcommandnr, ...);
if (bSendPacket) CL_SendMove();
else { m_NetChannel->SetChoked(); chokedcommands++; }
```

В CS:GO 2014 `bSendPacket` на стеке CreateMove:
- Хук CreateMove index 21, берем ebp
- `bSendPacket = *(bool*)(ebp - 0x1)` (в 2022 патче ebp-0x34, в 2014 - ebp-0x1)
- В проекте: `send_packet_ptr = *(PDWORD)(ebp) - 0x1`

### 2. ClientState (engine.dll)
Сигнатура: `A1 ? ? ? ? 8B 80 ? ? ? ? C3` -> mov eax, [ClientState]

Структура 2014-10-23:
```
0x9C: m_NetChannel
0x108: m_nSignonState (6 = FULL)
0x110: m_flNextCmdTime
0x4D2C: m_nLastOutgoingCommand
0x4D30: m_nChokedCommands (было 0x4CB0, в 2014 -> 0x4D30)
0x4D34: m_nLastCommandAck
```
INetChannel: `0x2C: m_nChokedPackets`

### 3. Fake Angle в 2014 - простой bSendPacket!
Ты прав, в 2014 нет LBY, фейк делается просто через bSendPacket:

```
if (bSendPacket) -> сервер видит real угол
else -> пакет чокнут, враги видят fake (предыдущий или чокнутый)
```

Классика 2014:
- Real = original.yaw
- Fake = original.yaw + 180 (можно 180, т.к. нет клампа LBY)
- Чокаем 1 тик fake, отправляем real -> десинк

### 4. Fake Lag (из твоего кода 2018, адаптировал под 2014)

Твой код 2018:
- ShouldFakeLag: moving, accelerate, air, unduck, weapon_activity, ladder
- DetermineFakeLagAmount: max, dynamic (64/speed), fluctuation (tick_count %40)
- HandleFakeLag: choke до AwaitingChoke, force send если >= LagLimit (14 в 2014, 62 в 2018)

Адаптация для 2014:
- Убрал tickbase, fake walking, mrx server check
- Max choke 14 (в 2014 sv_maxusrcmdprocessticks 16, безопасно 14)
- ShouldFakeLag: moving (vel>0.1), air (!FL_ONGROUND), ladder (movetype 9), unduck (duck_amount 0-1)
- DetermineFakeLagAmount: берет из конфига fakelag_ticks, clamp 1-14
- HandleFakeLag: choke до AwaitingChoke, force send если >=14, не чокаем гранаты при броске

### 5. Что сделал

- `packet_manager.hpp/cpp`:
  * ClientState + NetChannel реверс
  * bSendPacket control
  * ShouldFakeLag/DetermineFakeLagAmount/HandleFakeLag (адаптация твоего 2018 кода)

- `antiaim.hpp/cpp`:
  * УБРАЛ LBY BREAKER (ты прав, в 2014 его нет)
  * Простой bSendPacket фейк: BACKWARDS, SIDEWAYS, STATIC_180, JITTER, DESYNC (180), SPIN
  * Pitch: DOWN/UP/ZERO/JITTER

- `client.cpp`:
  * Получаем bSendPacket из ebp-0x1
  * Сначала HandleFakeLag, потом AntiAim, потом финальный apply

- `entity.hpp`:
  * Добавил m_velocity, m_move_type, m_duck_amount netvars для фейк лага

## Билд
```
python -m ziglang c++ -target x86-windows-gnu -shared -o csgosdk.dll 45 cpp -Isrc -include seh_fix.hpp -ld3d9 -ldwmapi ...
```
1.8MB PE32

## Дальше
- Точный max desync в 2014 был не 58, а можно 180 (нет LBY клампа)
- Добавить визуализацию real/fake
- Добавить больше условий для фейк лага (weapon activity и т.д. уже частично есть)

Спасибо что поправил про LBY - в 2014 его реально нет, я дебил.
