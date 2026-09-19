#include "../hooks.hpp"
#include "../../hacks/view.hpp"

void __stdcall hk_override_view(CViewSetup* viewsetup)
{
	static auto override_view_original = g_hooking_manager->client_mode_table->get_func_address<c_hooking::override_view_fn>(18);

	g_view->instance(viewsetup);
	override_view_original(viewsetup);
}

float __stdcall hk_get_viewmodel_fov()
{
	static auto get_viewmodel_fov_original = g_hooking_manager->client_mode_table->get_func_address<c_hooking::get_viewmodel_fov_fn>(35);

	return get_viewmodel_fov_original() + g_variables->visuals_viewmodel_fov;
}

void c_hooking::initialize_client_mode()
{
	client_mode_table->hook_function(reinterpret_cast<uintptr_t>(hk_override_view), 18);
	client_mode_table->hook_function(reinterpret_cast<uintptr_t>(hk_get_viewmodel_fov), 35);
}