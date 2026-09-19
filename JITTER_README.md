# Configurable Jitters - Radius + Max Speed (2014 CS:GO)

## Что сделано
User просил: джитеры которые настраиваются, именно радиус джитера настраивается, а сами джитеры просто дефольные джитеры на максимальной скорости.

### Config (config.hpp)
```cpp
int antiaim_jitter_range = 45; // yaw radius 0-180
int antiaim_jitter_range_pitch = 10; // pitch radius 0-89
bool antiaim_jitter_random = false; // random within radius vs fixed +-radius
int antiaim_jitter_speed = 1; // 1 = max speed every tick, 2 = every 2 ticks, etc.
```

### AntiAim Logic (antiaim.cpp)
- **Max speed**: jitter каждый тик (speed=1), дефольные джитеры на максимальной скорости
- **Configurable radius**: вместо фиксированных 90/-90 теперь `original.yaw ± m_jitter_range`
  - Например, range=45 → jitter между +45 и -45 каждый тик
  - range=180 → jitter 180/-180 (максимальный)
- **Random vs Fixed**:
  - Fixed: `m_jitter_side ? +range : -range` каждый тик
  - Random: `rand() % (range*2+1) - range` — случайный в пределах радиуса

#### Yaw Jitter
```cpp
bool should_jitter = (m_jitter_tick % m_jitter_speed) == 0;
if (should_jitter) m_jitter_side = !m_jitter_side;
float jitter_val = m_jitter_side ? m_jitter_range : -m_jitter_range;

if (send_packet) real.yaw = original.yaw + jitter_val;
else fake.yaw = original.yaw - jitter_val;
```
При `speed=1` — свитч каждый тик, максимальная скорость.

#### Pitch Jitter
Аналогично с `m_jitter_range_pitch`:
- Fixed: `pitch = ±range` (например, 10/-10) или `89±range`
- Random: random offset в пределах pitch radius
- Clamp к 89/-89

#### Другие моды тоже используют radius
- SIDEWAYS: `original ± m_jitter_range` вместо фиксированных 90
- SPIN: скорость спина `m_jitter_range * 0.5f` — чем больше радиус, тем быстрее спин
- DESYNC: `fake = original + m_jitter_range` вместо фиксированных 180

### GUI (content_tabs.cpp)
- Slider `Jitter Yaw Radius 0-180`
- Slider `Jitter Pitch Radius 0-89`
- Checkbox `Random Jitter (within radius)`
- Slider `Jitter Speed 1-10 (1=max speed every tick)`
- Текст: "Max speed = jitter every tick, default jitters"

### Пример настроек
- Малый джитер: range 15 → легкое дрожание ±15° каждый тик
- Средний: range 45 → стандартный ±45° каждый тик
- Максимальный: range 180 → 180/-180 каждый тик, максимальный десинк для 2014 (нет LBY клампа)
- Рандом: включить random + range 60 → каждый тик случайный угол в пределах ±60

Собрано: 45 cpp → 1.8MB DLL via zig.
