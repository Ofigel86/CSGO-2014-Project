#include "../hooks.hpp"
#include "../../hacks/view.hpp"

void __stdcall hk_override_view(CViewSetup* viewsetup)
{
    static auto override_view_original = g_hooking_manager->client_mode_table ?
        g_hooking_manager->client_mode_table->get_func_address<c_hooking::override_view_fn>(18) : nullptr;

    if (g_view && viewsetup)
    {
        __try { g_view->instance(viewsetup); } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    if (override_view_original)
    {
        __try { override_view_original(viewsetup); } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
}

float __stdcall hk_get_viewmodel_fov()
{
    static auto get_viewmodel_fov_original = g_hooking_manager->client_mode_table ?
        g_hooking_manager->client_mode_table->get_func_address<c_hooking::get_viewmodel_fov_fn>(35) : nullptr;

    float original_fov = 60.0f;
    if (get_viewmodel_fov_original)
    {
        __try { original_fov = get_viewmodel_fov_original(); } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    if (g_variables)
        return original_fov + static_cast<float>(g_variables->visuals_viewmodel_fov);

    return original_fov;
}

void c_hooking::initialize_client_mode()
{
    if (!client_mode_table)
        return;
    client_mode_table->hook_function(reinterpret_cast<uintptr_t>(hk_override_view), 18);
    client_mode_table->hook_function(reinterpret_cast<uintptr_t>(hk_get_viewmodel_fov), 35);
}
