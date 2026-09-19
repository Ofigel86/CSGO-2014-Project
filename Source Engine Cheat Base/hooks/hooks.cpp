#include "hooks.hpp"

c_hooking* g_hooking_manager = new c_hooking;

void c_hooking::initialize_virtual_tables()
{
	client_dll_table = new vmthook(reinterpret_cast<DWORD**>(g_interfaces->get_client_dll()));
	client_mode_table = new vmthook(reinterpret_cast<DWORD**>(g_interfaces->get_client_mode()));
	vgui_panel_table = new vmthook(reinterpret_cast<DWORD**>(g_interfaces->get_vgui_panel()));
	model_render_table = new vmthook(reinterpret_cast<DWORD**>(g_interfaces->get_model_render()));
	directx_device_table = new vmthook(reinterpret_cast<DWORD**>(g_interfaces->get_directx_device()));
}

void c_hooking::initialize_all_hooks()
{
	initialize_virtual_tables();

	initialize_client_dll();
	initialize_client_mode();
	initialize_vgui_panel();
	initialize_model_render();
	initialize_wndproc();
	initialize_directx_device();
}