#pragma once
#include "../interfaces/interfaces.hpp"

class c_visuals
{
public:
	void draw();
	void bomb_esp(c_c4* bomb);
	void bomb_timer(c_c4* bomb);
	void grenade_esp(c_base_entity* grenade);
	void grenade_tracers(c_base_entity* grenade);
	void draw_scope_lines();
};

extern c_visuals* g_visuals;