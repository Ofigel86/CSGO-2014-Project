# FULL REVERSE - CS:GO 2014-10-23 DLLs (client.dll 12MB, engine.dll 5.8MB, server.dll 9.7MB)

## Инструменты
- pefile, capstone, objdump, readelf
- Python скрипты для поиска паттернов, строк, интерфейсов
- Ручной анализ через hex + UC треды

---

## 1. engine.dll (5,842,432 bytes)

**PE Info:**
- Timestamp: 1414021165 → 2014-10-22 23:39:25 UTC
- Build string: `Exe build: 13:22:52 Oct 15 2014 (%i) (%i)` at 0x...
- ImageBase: 0x10000000, Entry: 0x3b6b97, SizeOfImage: 8630272
- Sections: .text (0x3f750c), .rdata (0x104642), .data (0x2d2068), _RDATA, .reloc
- Exports: 2 (CreateInterface at 0x102e5f40, ?F@@YAXPAPAVIEngineAPI@@@Z)
- Imports: 500+ funcs from USER32, KERNEL32, tier0 (172), steam_api (20), WSOCK32, etc.

**Interfaces (77):**
```
VAudio002, VBik001, VCLIENTENGINETOOLS001, VClient016, VClientDllSharedAppSystems001,
VClientEntityList003, VClientPrediction001, VDataCache003, VDebugOverlay004,
VEngineClient013, VEngineClientStringTable001, VEngineCvar007, VEngineEffects001,
VEngineModel016, VEngineRandom001, VEngineRenderView013, VEngineServer023,
VFileSystem017, VGUI_Panel009, VGUI_Surface031, VMaterialSystem080, VModelInfoClient004,
VPhysics031, VStudioRender026, etc.
```

**ClientState Reverse:**
- Sig: `A1 ? ? ? ? 8B 80 ? ? ? ? C3` → `mov eax, [ClientState]`
- Found at: pattern scan in .text
- Structure 2014-10-23:
```
0x00: pad
0x9C: INetChannel* m_NetChannel
0xA0: int m_nChallengeNr
0xA4: pad 100
0x108: int m_nSignonState (6=FULL)
0x110: float m_flNextCmdTime
0x114: float m_flNextCmdTime?
0x11C: int m_nCurrentSequence
0x174: int m_nDeltaTick
0x188: char m_szLevelName[260]
0x388: int m_nMaxClients
0x4D18: float m_flLastServerTickTime
0x4D1C: bool m_nInSimulation
0x4D2C: int m_nLastOutgoingCommand
0x4D30: int m_nChokedCommands (was 0x4CB0 old, now 0x4D30 in 2014)
0x4D34: int m_nLastCommandAck
0x4D38: int m_nCommandAck
0x4D3C: int m_nSoundSequence
0x4D90: Vector m_vecViewAngles
0x4E6C: CEventInfo* m_Events
```
- Size: 0x4E70

**INetChannel:**
```
0x18: m_nOutSequenceNr
0x1C: m_nInSequenceNr
0x2C: m_nChokedPackets (alternative choke counter)
```

**CL_Move Full Reverse (from engine.dll disasm + SDK 2007):**
```cpp
void CL_Move(float accumulated_extra_samples, bool bFinalTick) {
  if (!cl.IsConnected()) return;
  if (!Host_ShouldRun()) return;
  bool bSendPacket = true;
  if (demoplayer->IsPlayingBack()) {
    if (cl.ishltv) bSendPacket = false;
    else return;
  }
  if ((!cl.m_NetChannel->IsLoopback() || host_limitlocal.GetInt()) &&
      (net_time < cl.m_flNextCmdTime || !cl.m_NetChannel->CanPacket() || !bFinalTick))
    bSendPacket = false;

  if (cl.IsActive()) {
    int nextcommandnr = cl.lastoutgoingcommand + cl.chokedcommands + 1;
    g_ClientDLL->CreateMove(nextcommandnr, host_state.interval_per_tick - accumulated_extra_samples, !cl.IsPaused());
    if (demorecorder->IsRecording()) demorecorder->RecordUserInput(nextcommandnr);
    if (bSendPacket) CL_SendMove();
    else { cl.m_NetChannel->SetChoked(); cl.chokedcommands++; }
  }
  if (!bSendPacket) return;

  bool hasProblem = cl.m_NetChannel->IsTimingOut() && !demoplayer->IsPlayingBack() && cl.IsActive();
  if (hasProblem) { /* warning */ cl.ForceFullUpdate(); }

  if (cl.IsActive()) {
    NET_Tick mymsg(cl.m_nDeltaTick, host_frametime_unbounded, host_frametime_stddeviation);
    cl.m_NetChannel->SendNetMsg(mymsg);
  }
  cl.lastoutgoingcommand = cl.m_NetChannel->SendDatagram(NULL);
  cl.chokedcommands = 0;

  if (cl.IsActive()) {
    float commandInterval = 1.0f / cl_cmdrate->GetFloat();
    float maxDelta = min(host_state.interval_per_tick, commandInterval);
    float delta = clamp(net_time - cl.m_flNextCmdTime, 0.0f, maxDelta);
    cl.m_flNextCmdTime = net_time + commandInterval - delta;
  } else {
    cl.m_flNextCmdTime = net_time + (1.0f/5.0f);
  }
}
```
- `SetChoked` found via string, `CL_SendMove` found, `NET_SendPacket` found

**ConVars in engine.dll (619):**
`cl_allowdownload, cl_allowupload, cl_cmdrate, cl_updaterate, cl_interp, cl_predict, host_limitlocal, host_framerate, mat_*, net_*, sv_*, etc.`

---

## 2. client.dll (12,239,360 bytes)

**PE Info:**
- Timestamp: 1414102115 → 2014-10-23 22:08:35 UTC (exact target)
- ImageBase: 0x10000000, Entry: 0x7a842b, SizeOfImage: 79986688, SizeOfCode: 8381952
- Sections: .text (0x7fe49c), .rdata (0x20de68), .data (0x4130c08), _RDATA, .reloc
- Exports: 1 (CreateInterface at 0x106aec60)
- Imports: USER32, GDI32, steam_api (10), tier0 (77), vstdlib (17), KERNEL32 (134), etc.

**Interfaces (61+):**
```
VClient016 (at 0x8a3960), VClientDllSharedAppSystems001, VClientEntityList003,
VClientPrediction001, VDataCache003, VDebugOverlay004, VEngineCvar007,
VMaterialSystem080, VModelInfoClient004, etc.
VCWeaponHKP2000, VCWeaponM249, VCWeaponP228, VCWeaponP250, etc. (weapon factories)
```

**DataTables DT_ (271):**
```
DT_BaseEntity, DT_BasePlayer, DT_CSPlayer, DT_CSPlayerResource, DT_CSRagdoll,
DT_BaseCombatWeapon, DT_BaseCSGrenade, DT_WeaponCSBase, DT_BaseAnimating,
DT_BaseAnimatingOverlay, DT_BaseAttributableItem, DT_EconEntity, DT_WeaponAK47,
DT_WeaponAWP, etc. (full list 271)
```

**Netvars m_ (3511 unique):**
- Player: `m_iHealth, m_fFlags, m_vecOrigin[0-2], m_vecVelocity[0-2], m_aimPunchAngle, m_viewPunchAngle, m_bIsScoped, m_flFlashDuration, m_ArmorValue, m_bHasHelmet, m_hActiveWeapon, m_flDuckAmount, m_MoveType, m_flNextAttack, etc.`
- Weapon: `m_iClip1, m_flNextPrimaryAttack, m_iItemDefinitionIndex, m_bPinPulled, m_fThrowTime`
- Other: `m_Collision, m_MoveType, m_ModelName, m_FOV, m_Activity, m_AnimOverlay, etc.`

**CInput Reverse:**
- Pattern `8B 86 EC 00 00 00` (mov eax,[esi+0xEC]) — 70 occ → m_pCommands
- Pattern `8B 8E EC 00 00 00` — 21 occ
- `thirdperson` string at 0x8a2dc7
- Structure:
```cpp
class CInput {
  char pad[0xA4];
  bool m_fCameraInThirdPerson; // 0xA4
  bool m_fCameraMovingWithMouse; // 0xA5
  QAngle m_vecCameraOffset; // 0xA8
  bool m_fCameraDistanceMove; // 0xB4
  int m_nCameraOldX; // 0xB8
  int m_nCameraX; // 0xC0
  QAngle m_angPreviousViewAngles; // 0xCC
  float m_flLastForwardMove; // 0xE4
  int m_nClearInputState; // 0xE8
  CUserCmd* m_pCommands; // 0xEC
  CVerifiedUserCmd* m_pVerifiedCommands; // 0xF0
};
```
- GetUserCmd: `&m_pCommands[seq % 150]`

**Weapon VTable:**
- For 2014-10-23 VClient016: indices shifted +1 vs older
- Old: 458,459,460,430
- New: **459,460,461,431**
  - 459: get_inaccuracy
  - 460: get_spread
  - 461: update_accuracy_penalty
  - 431: get_weapon_data (weapon_info*)

**Classes (176):**
`CBaseEntity, CBasePlayer, CCSPlayer, CBaseCombatWeapon, CCSPlayerAnimState (old, not GO), CBasePlayerAnimState, CPlayerMove, CCSPlayerMove, etc.`
- **NO CCSGOPlayerAnimState** → confirms no LBY, old anim system
- **NO m_flLowerBodyYaw** in client.dll

**ConVars (1120 in client.dll):**
`cl_cmdrate, cl_updaterate, cl_interp, cl_crosshair*, cl_bob*, cl_csm_*, etc.`

---

## 3. server.dll (9,788,928 bytes)

**PE Info:**
- Timestamp: 1414102040 → 2014-10-23 22:07:20 UTC (same day as client)
- Sections: 5, x86
- Exports: 2, Imports: similar

**Classes:**
- `CCSPlayer (12 occ), CBasePlayer (12 occ), CCSPlayerAnimState (YES), CCSGOPlayerAnimState (NO), CBasePlayerAnimState (YES)`
- **NO m_flLowerBodyYaw, NO m_flLowerBodyYawTarget** → NO LBY in 2014
- `m_angEyeAngles[0], m_angEyeAngles[1]` — eye angles
- `CBasePlayer::ProcessUsercmds: too many cmds %i sent` — choke check
- `sv_maxusrcmdprocessticks` handling

**Interfaces (61):**
`VServerDllSharedAppSystems001, VEngineServer023, etc.`

**DataTables:** similar to client but server side

---

## 4. Packet Manager & Fake Angle (Implementation based on full reverse)

**Packet Manager (from engine.dll CL_Move + client.dll CInput):**
```cpp
// ClientState
CClientState** clientStatePtr = sigscan("A1 ? ? ? ? 8B 80 ? ? ? ? C3") +1
int choked = clientState->m_nChokedCommands; // 0x4D30
INetChannel* nc = clientState->m_NetChannel; // 0x9C
int chokedNet = nc->m_nChokedPackets; // 0x2C

// bSendPacket
bool* bSendPacket = (bool*)(ebp - 0x1); // from CreateMove hook

// Fake lag adapted from 2018 code
ShouldFakeLag:
  - moving: vel.Length2D()>0.1 && FL_ONGROUND
  - accelerate: prevVel < curVel
  - air: !(flags & FL_ONGROUND)
  - unduck: duck_amount 0-1 decreasing
  - ladder: movetype==9
  - weapon activity, etc.

DetermineFakeLagAmount: clamp(config.ticks,1,14) // 2014 limit 14

HandleFakeLag:
  if ShouldFakeLag -> AwaitingChoke = Determine()
  *bSendPacket = false
  if choked >= AwaitingChoke -> *bSendPacket = true
  if choked >=14 -> *bSendPacket = true (force)
  grenade check: if pin pulled and throwing -> send
```

**Fake Angle (2014, NO LBY):**
```cpp
real = original.yaw
fake = original.yaw + 180 // no clamp in 2014

if (choked <1) { yaw=fake; bSendPacket=false; }
else { yaw=real; bSendPacket=true; }
```

---

## 5. Build Verification

45 cpp files via zig `x86-windows-gnu`:
```
python -m ziglang c++ -target x86-windows-gnu -shared -o csgosdk.dll 45 files -ld3d9 -ldwmapi ...
```
Result: 1.8MB PE32 valid, includes all three DLL reverses.

## Conclusion
Full reverse done:
- engine.dll: CL_Move, ClientState, NetChannel, bSendPacket
- client.dll: CInput, weapon indices, netvars, DT_, interfaces, CreateMove
- server.dll: CCSPlayerAnimState (old), NO LBY, confirms 2014 has no desync 58/LBY breaker, only simple bSendPacket fake
- Packet manager + fake angle + fake lag implemented for 2014 build
