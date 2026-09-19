#include "config.hpp"

#include <Windows.h>

c_keybinds* g_keybinds = new c_keybinds;

void c_keybinds::handle_toggled_keybinds()
{
	static size_t last_time = 0;

	if (GetAsyncKeyState(g_variables->visuals_thirdperson_key))
	{
		if (GetTickCount() > last_time)
		{
			thirdperson = !thirdperson;
			last_time = GetTickCount() + 650;
		}
	}
	else if (!g_variables->visuals_thirdperson_key)
		thirdperson = true;
}

bool c_keybinds::get_thirdperson_state()
{
	return thirdperson;
}