# Cheat Improvement Roadmap - CS:GO 2014-10-23 (VClient016)

Сохранено все для дальнейшего улучшения чита. Билд: engine Oct 15 2014, client/server Oct 23 2014, 12MB client.dll.

## Что уже сохранено и работает

### 1. DLLs (оригинальные, для реверса)
- `engine.dll` 5.8MB (2014-10-22 23:39:25)
- `dlls/client.dll` 12MB (2014-10-23 22:08:35, VClient016)
- `dlls/server.dll` 9.7MB (2014-10-23 22:07:20)
- `client.dll`, `server.dll` в корне (дубли)

### 2. Реверс отчеты (сохранены)
- `FULL_REVERSE.md` — полный PE анализ всех 3 DLL: секции, экспорты, 77/61 интерфейсов, 271 DT_, 3511 netvars, 1120 cvars, паттерны CInput, weapon indices, ClientState
- `REVERSE_REPORT.md` — краткий отчет по ClientState, bSendPacket, LBY отсутствие
- `PACKET_MANAGER_RE.md` — детали пакет менеджера и фейк углов

### 3. Собранная DLL
- `Source Engine Cheat Base/compile/csgosdk.dll` 1.8MB (45 cpp, zig x86-windows-gnu)
- `compile/csgosdk.dll` 1.8MB (дубль)
- Собрано через `build_linux.sh` и `python -m ziglang c++ -target x86-windows-gnu`

### 4. Packet Manager (готов для фейк углов)
- `utilities/managers/packet_manager.hpp/cpp`
  * ClientState sig `A1 ? ? ? ? 8B 80 ? ? ? ? C3`
  * Offsets: `m_NetChannel 0x9C`, `m_nChokedCommands 0x4D30`, `m_nLastOutgoing 0x4D2C`, `INetChannel m_nChokedPackets 0x2C`
  * `bSendPacket` из `ebp-0x1` в CreateMove hook
  * `ShouldFakeLag`, `DetermineFakeLagAmount`, `HandleFakeLag` — адаптация твоего 2018 кода под 2014 (лимит 14, а не 62)
  * Netvars для фейк лага: `m_velocity`, `m_move_type`, `m_duck_amount` добавлены в `entity.hpp`

### 5. AntiAim / Fake Angle (готов, без LBY)
- `hacks/antiaim.hpp/cpp`
  * В 2014 НЕТ LBY — убран LBY breaker (ты прав)
  * Простой bSendPacket: `real=original`, `fake=original+180`
  * Моды: BACKWARDS, SIDEWAYS, STATIC_180, JITTER, DESYNC (180), SPIN
  * Pitch: DOWN/UP/ZERO/JITTER
  * Интеграция в `hooks/functions/client.cpp`: сначала HandleFakeLag, потом AntiAim, финальный apply bSendPacket

### 6. Порт под 2014-10-23
- CInput: `m_fCameraInThirdPerson 0xA4`, `m_pCommands 0xEC` (70 вхождений паттерна EC)
- Weapon: `get_inaccuracy 459`, `get_spread 460`, `update_accuracy_penalty 461`, `get_weapon_data 431` (+1 от старых)
- VClient016

## Что дальше улучшать (TODO)

### Из реверса DLLs — что еще можно вытащить
1. **CBasePlayerAnimState** (не GO) — в server.dll есть, но не GO. Нужно реверснуть его структуру для точного max desync (сейчас 180 статично, а можно вычислить)
2. **CCSPlayer::UpdateClientSideAnimation** — найти в client.dll, посмотреть как обновляются углы в 2014
3. **GetAllClasses** — уже есть, но можно дампнуть все ClientClasses с netvars для ESP
4. **ConVars** — 1120 в client.dll, 619 в engine.dll — можно сделать cvar manager для андетекта
5. **Material system** — для чамсов, уже есть база
6. **EngineTrace** — для autowall, уже частично

### Packet Manager улучшения
- [ ] Добавить динамический fakelag type (как в 2018: max, dynamic `64/speed`, fluctuation `tick%40`)
- [ ] Добавить variance (как в 2018)
- [ ] Добавить условия: `weapon_activity`, `m_flNextAttack`, `m_bPinPulled` (уже частично)
- [ ] Сделать `m_iOverrideLagAmount` для эксплоитов
- [ ] Добавить визуализацию choked commands в HUD

### Fake Angle улучшения (2014 специфика)
- [ ] В 2014 можно делать 180 без клампа, но можно также 360 spin — добавить
- [ ] Добавить `m_angEyeAngles` из server.dll реверса для проверки что сервер видит
- [ ] Добавить fake lag on shot (как в 2018 коде: `*bSendPacket = false` при `IN_ATTACK`)
- [ ] Добавить edge anti-aim (проверка стен через trace)

### Другие хаки
- [ ] Autowall — уже есть база, нужен `TraceToExit`
- [ ] Ragebot — добавить hitchance, pointscale
- [ ] Legitbot — smoothing
- [ ] ESP — добавить flags для helmet/kit/bomb, уже частично
- [ ] Chams — материал Regular/Metallic/Flat уже есть
- [ ] Skin changer — через EconEntity
- [ ] Glow — через GlowObjectManager

### Сборка
- [ ] GitHub Actions уже настроен в `build.yml.example` → нужно добавить через Web UI в main (из-за permission)
- [ ] `build_linux.sh` работает через zig, можно улучшить
- [ ] Добавить автоматический дамп netvars при запуске

## Как продолжать реверс

```bash
# Анализ DLLs
python3 -m pefile engine.dll
python3 reverse.py # скрипты в FULL_REVERSE.md

# Поиск паттернов
# CInput EC: 8B 86 EC 00 00 00
# ClientState: A1 ? ? ? ? 8B 80 ? ? ? ? C3
# bSendPacket: ebp-0x1 in CreateMove

# Строки
strings dlls/client.dll | grep m_
strings engine.dll | grep ClientState
```

## Сохранено в Git
- main: `d6e65dd` + `21febd2` + `a0fd843` + `3ce5d77` + `d73aa73`...
- arena: `ea8b0f8` (FULL_REVERSE)
- Все DLLs force-added, несмотря на .gitignore
- Все отчеты в .md

Можно дальше улучшать чит, все для этого сохранено.
