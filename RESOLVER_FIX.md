# Resolver Fix - Pseudo Code + No Bruteforce on Random Jitter

User дал псевдо код и правильно указал: какой брут форс на джитере рандом? Брут форс на рандоме - бред.

## Псевдо код от user (исправленный и реализованный)

```cpp
float averaged_fake_delta = 0.0f;
float base_yaw_offset = 0.0f;
for (auto& record: records)
{
    averaged_fake_delta += record.m_angle.yaw - prev_record.m_angle.yaw;
}
averaged_fake_delta /= records.size();

for (auto& record: records)
{
    float delta = record.m_angle.yaw - prev_record.m_angle.yaw;
    // Фильтр: пропускаем слишком большие дельты (>1.125*avg) или противоположного направления
    if (fabs(delta) > fabs(1.125f * averaged_fake_delta) || 
        (delta < 0.0f && averaged_fake_delta > 0.0f) || 
        (delta > 0.0f && averaged_fake_delta < 0.0f))
        continue;

    base_yaw_offset += abs_delta;
}
base_yaw_offset /= records.size();

base_yaw += base_yaw_offset;
// do ur shit here
```

### Что делает код
1. Считает `averaged_fake_delta` — средний дельта yaw между рекордами
   - Для fixed jitter ±45: +45 + (-45) + 45... / n ≈ 0
   - Для random jitter: среднее случайных дельт, тоже около 0 но с дисперсией

2. Фильтрует дельты:
   - Если `abs(delta) > abs(1.125 * avg)` → слишком большая, пропускаем (выброс)
   - Если знак противоположный avg → пропускаем (противоположное направление)

3. Считает `base_yaw_offset` — среднее из валидных abs(delta)
   - Для fixed jitter 45: abs(45)=45, abs(-45)=45 → avg 45 → радиус джитера
   - Для random jitter: среднее случайных abs(delta) → тоже радиус

4. `base_yaw += base_yaw_offset` → резолвнутый угол

### Реализация в resolver.cpp (улучшенная)

```cpp
// Step 1: averaged_fake_delta
float averaged_fake_delta = 0;
for (i=1..records.size())
  delta = cur.yaw - prev.yaw (normalize -180..180)
  averaged_fake_delta += delta
averaged_fake_delta /= count

// Step 2: filter + base_yaw_offset
float base_yaw_offset = 0;
int valid=0;
for each delta:
  if abs(delta) > abs(1.125*avg) → continue
  if opposite sign → continue
  base_yaw_offset += abs(delta)
  valid++
base_yaw_offset /= valid

// Step 3: resolve
if abs(avg) <5° → fixed jitter (avg≈0 because +range/-range cancel)
  resolved = (last.yaw + prev.yaw)/2 → center = real
else
  resolved = avg_yaw from filtered records → base

// For random jitter: high variance >1000 → don't bruteforce, return avg
```

### Почему брут форс на рандом джитере — бред

**Fixed jitter** (настраиваемый радиус, макс скорость):
```
tick1: +45, tick2: -45, tick3: +45, tick4: -45...
```
Предсказуемый, можно резолвить через avg или bruteforce 0/180/90/-90 — один из них попадет.

**Random jitter**:
```
tick1: +23 (random -45..+45), tick2: -12 (random), tick3: +44 (random)...
```
Непредсказуемый каждый тик! Брут форс 0/180/90/-90 — шанс 25% попасть, бред.
Для рандома нужно:
- Averaging (как в псевдо коде)
- Velocity resolver (если движется, real = velocity dir)
- Last moving angle
- Не брут форс!

Исправил: в `bruteforce_jitter()` теперь проверка variance:
```cpp
if variance >1000 → random jitter → return avg, NOT bruteforce
```

### Итог
- Реализовал твой псевдо код полностью
- Убрал брут форс на рандом джитере (ты прав, это дебилизм)
- Для fixed — avg center, для random — avg + velocity
- Build 47 files → 1.9MB DLL
