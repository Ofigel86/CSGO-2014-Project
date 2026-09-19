#pragma once
#include "../interfaces/interfaces.hpp"
#include "../utilities/managers/hooking_manager.hpp"

namespace csgo_window
{
	extern WNDPROC original_wndproc;
	extern HWND window;
}

class c_hooking
{
	void initialize_virtual_tables();

	void initialize_client_dll();
	void initialize_client_mode();
	void initialize_vgui_panel();
	void initialize_model_render();
	void initialize_directx_device();
	void initialize_wndproc();
public:
	void initialize_all_hooks();

	vmthook* client_dll_table;
	vmthook* client_mode_table;
	vmthook* vgui_panel_table;
	vmthook* model_render_table;
	vmthook* directx_device_table;

	using create_move_fn = void(__thiscall*)(void*, int, float, bool);
	using paint_traverse_fn = void*(__thiscall*)(void*, unsigned int, bool, bool);
	using draw_model_execute_fn = void(__thiscall*)(void*, void*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);
	using frame_stage_notify_fn = void(__thiscall*)(void*, ClientFrameStage_t);
	using override_view_fn = void(__stdcall*)(CViewSetup*);
	using do_post_screen_effects_fn = void(__thiscall*)(void*, CViewSetup*);
	using get_viewmodel_fov_fn = float(__stdcall*)();
	using end_scene_fn = long(__stdcall*)(IDirect3DDevice9*);
};

extern c_hooking* g_hooking_manager;