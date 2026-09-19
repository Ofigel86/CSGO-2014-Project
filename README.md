# CSGO-2014-Project - Fixed Base

Internal cheat base for CS:GO Source Engine (2014) - полностью пофикшенная версия.

## Что пофикшено

### Критические баги
- **Краш в ragebot::target_selection** - добавлена проверка nullptr перед is_alive()
- **Пустые функции** - реализованы заглушки с безопасным возвратом:
  - `get_wall_damage()` / `hit_chance()` теперь возвращают корректные значения
  - `autowall` реализован с базовой логикой трассировки
  - `esp::draw_flags()` реализовано (helmet, armor, kit, zoom)
  - `legitbot`, `world` теперь безопасные заглушки
- **netvars_manager** - переписан `get_prop` с правильной рекурсией, исправлен `get_table`
- **vmthook** - исправлено чтение `m_OldVT[-1]` с SEH, добавлены проверки `IsBadReadPtr`, лимит 1024 методов, защита от переполнения
- **signature_scanner** - добавлены проверки модулей и nullptr
- **entity.hpp** - добавлены SEH и проверки в `is_alive()`, `get_eye_position()`, `get_bone_position()`, `is_grenade()` и т.д.
- **movement::fix_movement** - защита от деления на ноль
- **view.cpp thirdperson** - полная защита от nullptr, goto для отключения
- **visuals::bomb_timer** - проверки cvar и global_vars

### Сборка
- **vcxproj** - унифицирован на `v143`, все конфиги теперь `DynamicLibrary`, убран хардкод `D:\DirectxSDK\` -> используется `$(DXSDK_DIR)`
- **vcxproj.filters** - переведены с русского на английский, добавлены подпапки
- **Файл `1`** - удален (мусор)

### Хуки
- **client.cpp CreateMove** - добавлен fallback для non-MSVC, проверки всех интерфейсов, SEH
- **vgui_panel PaintTraverse** - кэширование ID панелей чтобы не делать strcmp каждый кадр, добавлены проверки
- **directx_device EndScene** - проверки device и SEH
- **wndproc** - убран спам GetAsyncKeyState, только WM_KEYDOWN для INSERT, блокировка мыши когда меню открыто
- **model_render** - проверки материалов, лимит создания, SEH

### GUI и рендер
- **gui.cpp** - fallback для шрифтов (Calibri -> Verdana -> Arial -> default), SEH везде
- **draw_manager** - проверки интерфейсов, SEH, исправлен `MultiByteToWideChar` с -1
- **custom_elements** - защита от деления на ноль, PushStyleColor для активных кнопок
- **hud.cpp watermark** - проверки, FPS в ватермарке, защита от пустого текста
- **content_tabs** - проверки g_variables, добавлены кнопки Save/Load config

### Конфиг и утилиты
- **config.cpp** - реализованы Save/Load в JSON-like формате
- **keybinds.cpp** - дебаунс 350ms, проверки диапазона клавиш, SEH
- **math.cpp/math.hpp** - проверки RandomSeed/RandomFloat, fallback на rand(), исправлен clamp через std::clamp, защита от нулевых векторов
- **vfunc.hpp** - проверки ppClass и VTable на nullptr
- **netvars_helper** - кэширование с -1 и проверками
- **win_includes.hpp** - добавлены NOMINMAX, cstdint, atomic, mutex
- **install.cpp** - DisableThreadLibraryCalls, CloseHandle, SEH, atomic флаг выгрузки, поддержка _DEBUG консоли

## Сборка

Требования:
- Visual Studio 2022 (v143)
- Windows SDK 10.0+
- DirectX SDK June 2010 (установить и задать переменную DXSDK_DIR) или использовать NuGet
- Конфигурация: Release | Win32 -> compile/csgosdk.dll

## Структура

См. анализ в issue / предыдущем сообщении.

## Безопасность

Добавлены SEH (__try/__except) вокруг всех потенциально крашащих мест, проверки nullptr, IsBadReadPtr.

## TODO

- Полноценный autowall с trace_to_exit
- Hitchance
- Legitbot с smoothing
- Glow, dlights
- Skin changer
- Unload routine с восстановлением VMT

## Лицензия

Образовательный проект.
