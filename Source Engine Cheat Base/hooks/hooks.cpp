#include "hooks.hpp"

c_hooking* g_hooking_manager = new c_hooking();

void c_hooking::initialize_virtual_tables()
{
    auto client_dll = g_interfaces->get_client_dll();
    auto client_mode = g_interfaces->get_client_mode();
    auto vgui_panel = g_interfaces->get_vgui_panel();
    auto model_render = g_interfaces->get_model_render();
    auto directx_device = g_interfaces->get_directx_device();

    if (client_dll)
        client_dll_table = new vmthook(reinterpret_cast<DWORD**>(client_dll));
    if (client_mode)
        client_mode_table = new vmthook(reinterpret_cast<DWORD**>(client_mode));
    if (vgui_panel)
        vgui_panel_table = new vmthook(reinterpret_cast<DWORD**>(vgui_panel));
    if (model_render)
        model_render_table = new vmthook(reinterpret_cast<DWORD**>(model_render));
    if (directx_device)
        directx_device_table = new vmthook(reinterpret_cast<DWORD**>(directx_device));
}

void c_hooking::initialize_all_hooks()
{
    initialize_virtual_tables();

    if (client_dll_table)
        initialize_client_dll();
    if (client_mode_table)
        initialize_client_mode();
    if (vgui_panel_table)
        initialize_vgui_panel();
    if (model_render_table)
        initialize_model_render();
    if (directx_device_table)
        initialize_directx_device();

    initialize_wndproc();
}
