#pragma once
#include "../interfaces/interfaces.hpp"

class c_world
{
public:
	void player_dlights();
	void player_glow();
};

extern c_world* g_world;