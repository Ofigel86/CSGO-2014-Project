#pragma once
#include "../interfaces/interfaces.hpp"

class c_movement
{
public:
	void bunny_hop(CUserCmd* command);
	void auto_strafe(CUserCmd* command);
	void fix_movement(CUserCmd* command);
};

extern c_movement* g_movement;
