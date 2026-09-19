#include "config.hpp"

#include <windows.h>

c_keybinds* g_keybinds = new c_keybinds();

void c_keybinds::handle_toggled_keybinds()
{
    if (!g_variables)
        return;

    static DWORD last_time = 0;
    DWORD current_time = GetTickCount();

    // Debounce handling
    if (current_time < last_time)
        last_time = 0;

    int key = 0;
    __try {
        key = g_variables->visuals_thirdperson_key;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return; }

    if (key > 0 && key < 256)
    {
        __try {
            if (GetAsyncKeyState(key) & 0x8000)
            {
                if (current_time > last_time)
                {
                    thirdperson = !thirdperson;
                    last_time = current_time + 350; // 350ms debounce
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
    else if (key == 0)
    {
        // If no key set, thirdperson follows the config bool
        __try {
            thirdperson = g_variables->visuals_thirdperson;
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
}

bool c_keybinds::get_thirdperson_state()
{
    __try {
        return thirdperson;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}
