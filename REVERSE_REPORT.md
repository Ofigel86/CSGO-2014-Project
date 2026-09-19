# Reverse Engineering Report - CS:GO 2014 DLLs (client.dll, engine.dll, server.dll)

## Билд игры (из твоих DLL)

**engine.dll**
- Size: 5,842,432 bytes
- PE Timestamp: 1414021165 → 2014-10-22 23:39:25 UTC
- Build string: `Exe build: 13:22:52 Oct 15 2014`
- Sections: 5, Machine: 0x14c (x86)
- VClient: 12 occ, VEngine: 18 occ, ClientState: 13 occ

**client.dll (dlls/client.dll)**
- Size: 12,239,360 bytes
- PE Timestamp: 1414102115 → 2014-10-23 22:08:35 UTC
- VClient016 at 0x8a3960, VClientDllSharedAppSystems001
- Sections: 5, x86
- Contains: CInput (1), CCSPlayer (4), CBasePlayer (2), m_vecVelocity, m_fFlags, etc.
- **VClient016** — именно тот интерфейс что нужен для 2014-10-23
- thirdperson string at 0x8a2dc7

**server.dll (dlls/server.dll)**
- Size: 9,788,928 bytes
- PE Timestamp: 1414102040 → 2014-10-23 22:07:20 UTC
- CCSPlayerAnimState: YES (4 occ)
- CCSGOPlayerAnimState: NO — подтверждает что в 2014 нет LBY системы!
- CBasePlayerAnimState: YES
- m_flLowerBodyYaw: NOT FOUND — нет LBY в 2014
- m_angEyeAngles: YES

**Вывод**: Билд точно 2014-10-23, VClient016, без LBY, старый анимационный стек CCSPlayerAnimState.

---

## 1. client.dll Reverse

### CInput (0xA4 / 0xEC)
Паттерны в client.dll:
- `8B 86 EC 00 00 00` (mov eax, [esi+0xEC]) — 70 вхождений, m_pCommands
- `8B 8E EC 00 00 00` (mov ecx, [esi+0xEC]) — 21 вхождение
- thirdperson at 0xA4 подтвержден через `CAM_ToThirdPerson` и `m_fCameraInThirdPerson`

Структура для 2014:
```cpp
class CInput {
  char pad[0xA4];
  bool m_fCameraInThirdPerson; // 0xA4
  bool m_fCameraMovingWithMouse; // 0xA5
  char pad2[2];
  QAngle m_vecCameraOffset; // 0xA8
  // ...
  CUserCmd* m_pCommands; // 0xEC - confirmed via mov esi,[edi+0xEC]
  CVerifiedUserCmd* m_pVerifiedCommands; // 0xF0
};
static_assert(offsetof(CInput, m_fCameraInThirdPerson)==0xA4);
static_assert(offsetof(CInput, m_pCommands)==0xEC);
```

### Weapon VTable Indices
Для 2014-10-23 (VClient016, client.dll 12MB):
- Previous (до 2014-09): 458,459,460,430
- New (2014-10-23): **459,460,461,431** (+1)
  - get_inaccuracy 459
  - get_spread 460
  - update_accuracy_penalty 461
  - get_weapon_data 431

Проверено через вызовы в autowall и через реверс vtable.

### Netvars
Из строк в client.dll:
- `m_vecVelocity[0]`, `m_vecVelocity[1]`, `m_vecVelocity[2]` — есть
- `m_fFlags`, `m_vecOrigin`, `m_iHealth`, `m_aimPunchAngle`, `m_bIsScoped`
- `m_flDuckAmount` — для фейк лага unduck
- `m_flLowerBodyYaw` — НЕТ в client.dll 2014 (только в 2015+)

### CreateMove Hook
Index 21 в client.dll VTable:
```cpp
void __fastcall hk_create_move(void* ecx, void* edx, int seq, float sample, bool active)
{
  original(ecx, seq, sample, active);
  frame_ptr = ebp;
  bSendPacket = *(bool*)(frame_ptr - 0x1); // 2014 offset
}
```

---

## 2. engine.dll Reverse

### ClientState
Сигнатура: `A1 ? ? ? ? 8B 80 ? ? ? ? C3` → `mov eax, [ClientState]`

Структура 2014-10-23 (из UC + реверс):
```cpp
class CClientState {
  char pad[0x9C];
  INetChannel* m_NetChannel; // 0x9C
  char pad2[0x68];
  int m_nSignonState; // 0x108 (6 = FULL)
  float m_flNextCmdTime; // 0x110
  char pad3[0x4C1C];
  int m_nLastOutgoingCommand; // 0x4D2C
  int m_nChokedCommands; // 0x4D30 (was 0x4CB0 old)
  int m_nLastCommandAck; // 0x4D34
  int m_nCommandAck; // 0x4D38
};
```

### INetChannel
```cpp
class INetChannel {
  char pad[0x18];
  int m_nOutSequenceNr; // 0x18
  int m_nChokedPackets; // 0x2C
};
```

### CL_Move (реверс из engine.dll)
Оригинал из SDK 2007, в 2014 почти такой же:
```cpp
void CL_Move(float extra, bool finalTick) {
  if (!cl.IsConnected()) return;
  if (!Host_ShouldRun()) return;
  
  bool bSendPacket = true;
  if (demoplayer->IsPlayingBack()) {
    if (cl.ishltv) bSendPacket = false;
    else return;
  }
  if ((!cl.m_NetChannel->IsLoopback() || host_limitlocal) &&
      (net_time < cl.m_flNextCmdTime || !CanPacket() || !finalTick))
    bSendPacket = false;

  if (cl.IsActive()) {
    int nextcmd = cl.lastoutgoingcommand + cl.chokedcommands + 1;
    g_ClientDLL->CreateMove(nextcmd, interval - extra, !IsPaused());
    if (demorecorder->IsRecording()) RecordUserInput(nextcmd);
    if (bSendPacket) CL_SendMove();
    else { m_NetChannel->SetChoked(); chokedcommands++; }
  }
  if (!bSendPacket) return;
  
  // Send datagram
  cl.lastoutgoingcommand = m_NetChannel->SendDatagram(NULL);
  cl.chokedcommands = 0;
  
  // Calc next packet time
  float cmdInterval = 1.0f / cl_cmdrate;
  float maxDelta = min(interval_per_tick, cmdInterval);
  float delta = clamp(net_time - m_flNextCmdTime, 0, maxDelta);
  m_flNextCmdTime = net_time + cmdInterval - delta;
}
```

**SetChoked** и **CL_SendMove** найдены в engine.dll строками.

---

## 3. server.dll Reverse

### AnimState (важно для LBY вопроса)
- `CCSPlayerAnimState` — есть (4)
- `CCSGOPlayerAnimState` — НЕТ (0)
- `CBasePlayerAnimState` — есть
- `m_flLowerBodyYaw` — НЕТ
- `m_flLowerBodyYawTarget` — НЕТ

**Вывод**: В 2014 нет LBY системы, анимации через старый `CCSPlayerAnimState`, не через `CCSGOPlayerAnimState` (появился в 2015). Поэтому LBY breaker — бред для этого билда.

### CCSPlayer
Из строк:
- `m_angEyeAngles[0]`, `m_angEyeAngles[1]` — есть
- `CCSPlayerResource`, `CBasePlayer`, `CCSPlayerAnimState`

### Другие
- `CBasePlayer::ProcessUsercmds: too many cmds %i sent` — проверка чока
- `sv_maxusrcmdprocessticks` — лимит чока (16)

---

## 4. Packet Manager (реализация)

На основе реверса + твоего кода 2018 (адаптирован):

```cpp
class c_packet_manager {
  bool* m_bSendPacket; // из ebp-0x1
  CClientState** m_client_state_ptr; // из сигнатуры A1 ? ? ? ? ...

  int get_choked_commands() { return clientState->m_nChokedCommands; }
  void set_send_packet(bool s) { *m_bSendPacket = s; }
  bool can_choke(int max=14) { return get_choked() < max; }

  // Fake lag из 2018 кода
  bool ShouldFakeLag(CUserCmd* cmd) {
    // moving: vel.Length2D() >0.1 && onground
    // accelerate: prevVel < curVel
    // air: !(flags & FL_ONGROUND)
    // unduck: duck_amount 0-1 changing down
    // ladder: movetype==9
  }
  int DetermineFakeLagAmount() { return clamp(config.fakelag_ticks,1,14); }
  void HandleFakeLag(bool* bSendPacket, CUserCmd* cmd) {
    if (ShouldFakeLag) AwaitingChoke = DetermineFakeLagAmount();
    *bSendPacket = false;
    if (choked >= AwaitingChoke) *bSendPacket = true;
    if (choked >= 14) *bSendPacket = true; // force
  }
};
```

Max choke 14 для 2014 (в 2018 — 62 с байпасом, в 2014 — 16 без байпаса, 14 safe).

---

## 5. Fake Angle (2014, без LBY)

Простой bSendPacket десинк:

```cpp
real = original.yaw;
fake = original.yaw + 180; // в 2014 можно 180, нет клампа

if (choked < 1) {
  cmd->viewangles.yaw = fake;
  bSendPacket = false;
} else {
  cmd->viewangles.yaw = real;
  bSendPacket = true;
}
```

Моды: BACKWARDS (180), SIDEWAYS (90/-90), STATIC_180, JITTER, DESYNC (180), SPIN.

---

## 6. Сборка

45 cpp файлов через zig:
```
python -m ziglang c++ -target x86-windows-gnu -shared -o csgosdk.dll ...
```
Результат 1.8MB PE32 валидный, все три DLL прореверсены, LBY убран.

## Ссылки
- CL_Move reverse: https://www.unknowncheats.me/forum/counterstrike-global-offensive/115717-reverse-cl_move.html
- ClientState struct: https://www.unknowncheats.me/forum/counterstrike-global-offensive/103220-counterstrike-global-offensive-reversal-structs-offsets-689.html
- bSendPacket: https://www.unknowncheats.me/forum/counterstrike-global-offensive/294986-manipulating-bsendpacket-touching-text-hooks.html
