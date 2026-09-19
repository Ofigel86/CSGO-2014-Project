#pragma once
#include "../interfaces/interfaces.hpp"

class c_esp
{
	Vector draw_top_3d;
	Vector draw_pos_3d;

	void get_render_bounds(c_cs_player* player);

	void draw_name(c_cs_player* player);
	void draw_bounding_box(c_cs_player* player);
	void draw_health_bar(c_cs_player* player);
	void draw_flags(c_cs_player* player);
public:
	void draw();
};

extern c_esp* g_esp;