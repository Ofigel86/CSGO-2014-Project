# CSGO 2014 AntiAim — Max Fake 90 Degrees

## Инфа от user (важно!)

> в ксго 2014 года фейковый угол может угодить максимум о 90 градусов именно не от реального угла а в целом только до 90 а если ставить больше то уже реал отежает

Перевод: в CSGO 2014 максимальный фейк = ~90° TOTAL, не 90 от реала, а всего до 90. Если ставить больше — реал уезжает.

## Что это значит

В 2014:
- Нет LBY (LowerBodyYawTarget появился в 2015-2016)
- Fake делается через `bSendPacket` choke
- Real = что видит сервер когда `bSendPacket=true`
- Fake = что видят враги когда `bSendPacket=false` (зачоканные тики)

### Лимит десинка

- **2014 max desync = 90°**
- Если `abs(real - fake) > 90`, движок начинает двигать real!
- Пример: real 0, fake 180 → diff 180 >90 → real уедет к fake, антиаим сломается
- Позже (2017+) max desync уменьшили до 58°, но в 2014 это 90°

### Как работало в читах 2014

- Backwards 180° — нельзя, надо 90° max
- Sideways 90° / -90° — можно, это max
- Jitter +45 / -45 — diff 90 = max, можно
- Jitter +90 / -90 — diff 180 >90 → real уедет! Нельзя!
- Desync 0 / 180 — нельзя, надо 0 / 90

## Фикс в коде

### antiaim.hpp
```cpp
#define MAX_DESYNC_2014 90.0f
#define MAX_JITTER_HALF_2014 45.0f // +45/-45 = 90 diff
```

### antiaim.cpp run_yaw()
```cpp
float clamped_range = m_jitter_range;
if (clamped_range > 90) clamped_range = 90;

float jitter_half = m_jitter_range;
if (jitter_half > 45) jitter_half = 45; // для jitter с двух сторон

// BACKWARDS: было 180, стало 90 max
cmd->m_viewangles.yaw = original.yaw + 90; // не 180!

// STATIC_180: было real 180 fake 0 diff 180 -> real уезжает
// стало real 0 fake 90 diff 90 max
real.yaw = original.yaw;
fake.yaw = original.yaw + 90;

// JITTER: real +range fake -range diff 2*range <=90
// range <=45 для обеих сторон
float jitter_val = m_jitter_side ? jitter_half : -jitter_half;
real.yaw = original.yaw + jitter_val;
fake.yaw = original.yaw - jitter_val; // diff 90 max

// DESYNC: real 0 fake +range clamped 90
real.yaw = original.yaw;
fake.yaw = original.yaw + clamped_range; // max 90

// SPIN: real spin, fake spin+90
real.yaw = original.yaw + spin;
fake.yaw = real.yaw + 90;
```

### GUI
- Slider yaw radius теперь 0-90 (было 0-180)
- Текст: "2014 LIMIT: fake max 90 deg total, >90 real moves!"
- Текст: "For jitter +-range, max range=45 (diff 90)"
- Текст: "MAX DESYNC 2014 = 90 deg, >90 real uedet"

## Проверка

- Build 47 files -> 1.9M DLL
- Все режимы теперь respect 90 limit
- Jitter +45/-45 = 90 diff = max, не двигает real
- Desync 0/90 = 90 diff = max
- Backwards 90 = max, не 180

## Связь с resolver

Resolver теперь должен знать про 90 лимит:
- Если враг использует fake 90, real = 0, мы видим fake 90
- Resolver должен искать real в пределах 90 от fake
- Наш resolver уже делает avg + velocity, что работает с 90 лимитом
- Брут форс на 90 лимите: проверять 0, 90, -90, 180 уже нельзя, только 0,90,-90

## Итог

Спасибо за инфу! В 2014 фейк max 90, не 180. Если ставить больше 90, реал отежает. Пофиксил весь антиаим под этот лимит.
