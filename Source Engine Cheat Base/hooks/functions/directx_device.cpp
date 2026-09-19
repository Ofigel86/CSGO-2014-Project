#include "../hooks.hpp"
#include "../../gui/gui.hpp"

HRESULT __stdcall hk_end_scene(IDirect3DDevice9* device)
{
    static auto end_scene_original = g_hooking_manager->directx_device_table ?
        g_hooking_manager->directx_device_table->get_func_address<c_hooking::end_scene_fn>(42) : nullptr;

    if (!end_scene_original || !device)
        return D3D_OK;

    if (!g_gui)
        return end_scene_original(device);

    __try {
        g_gui->initialize(device);
        g_gui->setup_gui_style();
        g_gui->begin_draw_frame();
        g_gui->draw_hud();
        g_gui->draw_menu();
        g_gui->end_draw_frame();
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    __try {
        return end_scene_original(device);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return D3D_OK;
    }
}

void c_hooking::initialize_directx_device()
{
    if (!directx_device_table)
        return;
    directx_device_table->hook_function(reinterpret_cast<uintptr_t>(hk_end_scene), 42);
}
