#pragma once

#include "drawing/imgui.h"
#include "drawing/imgui_impl_dx9.h"
#include "drawing/imgui_impl_win32.h"
#include "drawing/imgui_internal.h"

#include <d3d9.h>

class c_gui
{
	bool draw_tab_button(const char* name, bool active, bool last);
	bool draw_subtab_button(const char* name, bool active, int subtabs_count, bool last);
	bool draw_hotkey_button(const char* name, int* key);
	float calculate_subtab_button_size(int count);
	float calculate_tab_button_size();

	void draw_rage_tab();
	void draw_legit_tab();
	void draw_esp_tab();
	void draw_misc_tab();
	void draw_skins_tab();

	void draw_watermark();
	void draw_keybinds();

	void draw_background();
	void draw_tabs_bar();
	void draw_content();
public:
	void initialize(PDIRECT3DDEVICE9 device);
	void setup_gui_style();
	void begin_draw_frame();
	void end_draw_frame();

	void unlock_cursor();

	void draw_menu();
	void draw_hud();

	bool menu_state = false;
	bool initialized_directx = false;

	int current_tab = -1;
	int tabs_count = -1;

	int current_subtab_rage = 0;
	int current_subtab_visuals = 0;
	int current_subtab_misc = 0;

	ImVec2 menu_size = ImVec2(620, 485);
};

extern c_gui* g_gui;