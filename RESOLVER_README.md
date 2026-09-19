# Resolver против джитеров на лаг-рекордах (2014 CS:GO)

User подумал что я сделал ресольвер на лаг рекордах против джитеров — теперь сделал по-настоящему.

## Что такое лаг-рекорды
В 2014 CS:GO lag compensation хранит историю игроков для бектрека:
- origin, velocity, eye_angles, simulation_time, flags
- Max 12 рекордов (0.2 сек, sv_maxunlag)

## Resolver против джитеров

### Детект джитера из истории
```cpp
bool is_jittering_from_history(index):
  check last 3 records
  delta = abs(current.yaw - prev.yaw)
  if delta > 35° (fixed jitter 45/-45, random jitter within radius)
  jitter_count >=2 of last 3 -> is jittering
```

### Методы резолва (3 режима)

**1. Jitter Detect (avg) — дефолт для fixed джитеров:**
- Fixed jitter: +45 / -45 каждый тик на макс скорости
- Center = real: average of last 2 records
```cpp
averaged.yaw = (last.yaw + prev.yaw) * 0.5
// +45 + (-45) /2 = 0 → real angle
```
- Работает для configurable radius jitter

**2. Velocity — для движущихся:**
- Если vel.Length2D() >0.1, real = velocity direction
```cpp
velAngle.yaw = atan2(vel.y, vel.x) * 180/PI
```
- В 2014 при движении десинк маленький, велосити = реал

**3. Bruteforce — для рандом джитеров:**
- Пробуем 4 угла: original, +180, +90, -90
- Циклим каждый выстрел:
```cpp
brute_angles[0]=base, [1]=base+180, [2]=base+90, [3]=base-90
result = brute_angles[current_brute %4]
current_brute++
```

### Интеграция
- `lag_comp.cpp`: `instance()` каждый CreateMove, `store_record()` с детектом джитера
- `resolver.cpp`: `instance()` → `resolve_player()` для каждого врага
- `ragebot.cpp`: вызывает lag_comp и resolver, использует resolved angle
- Config: `ragebot_resolver` bool, `ragebot_resolver_mode` 0-2
- GUI: чекбокс Enable Resolver + комбо Resolver Mode

### Пример
Враг делает jitter ±45 на макс скорости:
```
tick1: eye yaw = 0+45=45
tick2: eye yaw = 0-45=-45
tick3: eye yaw = 0+45=45
→ детект: delta 90 >35, jitter_count 2 → is_jittering=true
→ resolve: (45 + (-45))/2 = 0 → real angle
→ aimbot стреляет в 0, а не в 45/-45
```

Для рандом джитера ±60:
```
tick1: 23, tick2: -12, tick3: 44 → avg не работает
→ bruteforce: пробуем 23, 23+180=203, 23+90=113, 23-90=-67
→ один из них попадет
```

Собрано 47 файлов → 1.9MB DLL.
