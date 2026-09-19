#pragma once
#include "../../interfaces/interfaces.hpp"

class c_draw_manager
{
public:
	void initialize();

	void draw_filled_rect(float x, float y, float width, float height, Color color);
	void draw_outlined_rect(float x, float y, float width, float height, Color color);
	void draw_line(int x, int y, int x2, int y2, Color color);
	void draw_text(vgui::HFont font, int x, int y, Color color, DWORD flags, const char* msg, ...);
	Vector2D get_text_size(vgui::HFont font, const char* msg, ...);
	bool world_to_screen(const Vector& world, Vector& screen);

	vgui::HFont create_font(const char* name, int size, int weight, DWORD flags);

	int screen_width;
	int screen_height;

	vgui::HFont verdana_font;
	vgui::HFont small_font;
};

extern c_draw_manager* g_render;