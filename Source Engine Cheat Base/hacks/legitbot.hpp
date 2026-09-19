#pragma once
#include "../interfaces/interfaces.hpp"

class c_legitbot
{
public:
	void instance(CUserCmd* command);
};

extern c_legitbot* g_legitbot;