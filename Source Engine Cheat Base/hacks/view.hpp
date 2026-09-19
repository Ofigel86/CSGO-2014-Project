#pragma once
#include "../interfaces/interfaces.hpp"

class c_view
{
public:
	void instance(CViewSetup* viewsetup);
	void world_fov_changer(CViewSetup* viewsetup);
	void third_person();
};

extern c_view* g_view;