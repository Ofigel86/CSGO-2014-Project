#pragma once
#include "../interfaces/interfaces.hpp"

class c_nospread
{
public:
	void instance(CUserCmd* command);

	void compensate_spread(CUserCmd* command);
	void compensate_recoil(CUserCmd* command);
};

extern c_nospread* g_nospread;