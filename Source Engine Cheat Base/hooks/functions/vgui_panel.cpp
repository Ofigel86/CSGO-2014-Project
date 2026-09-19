#include "../hooks.hpp"
#include "../../hacks/esp.hpp"
#include "../../hacks/visuals.hpp"
#include "../../gui/gui.hpp"

void __fastcall hk_paint_traverse(void* ecx, void* edx, unsigned int panel, bool force_repaint, bool allow_force)
{
	static auto paint_traverse_original = g_hooking_manager->vgui_panel_table->get_func_address<c_hooking::paint_traverse_fn>(41);

	if (!strcmp("HudZoom", g_interfaces->get_vgui_panel()->GetName(panel)))
		if (g_variables->removals_scope)
			return;

	paint_traverse_original(ecx, panel, force_repaint, allow_force);

	if (!strcmp(g_interfaces->get_vgui_panel()->GetName(panel), "MatSystemTopPanel"))
	{
		if (!g_context->initialised_fonts)
		{
			g_render->initialize();
			g_context->initialised_fonts = true;
		}

		g_gui->unlock_cursor();

		g_visuals->draw();
		g_esp->draw();
	}
}

void c_hooking::initialize_vgui_panel()
{
	vgui_panel_table->hook_function(reinterpret_cast<uintptr_t>(hk_paint_traverse), 41);
}