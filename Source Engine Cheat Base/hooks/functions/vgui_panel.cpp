#include "../hooks.hpp"
#include "../../hacks/esp.hpp"
#include "../../hacks/visuals.hpp"
#include "../../gui/gui.hpp"

// Cache panel IDs to avoid strcmp every frame
static unsigned int g_mat_system_top_panel = 0;
static unsigned int g_hud_zoom_panel = 0;
static bool g_panels_cached = false;

void __fastcall hk_paint_traverse(void* ecx, void* edx, unsigned int panel, bool force_repaint, bool allow_force)
{
    static auto paint_traverse_original = g_hooking_manager->vgui_panel_table ?
        g_hooking_manager->vgui_panel_table->get_func_address<c_hooking::paint_traverse_fn>(41) : nullptr;

    if (!paint_traverse_original)
        return;

    // Cache panel IDs on first run
    if (!g_panels_cached && g_interfaces && g_interfaces->get_vgui_panel())
    {
        __try {
            auto panel_interface = g_interfaces->get_vgui_panel();
            const char* name = panel_interface->GetName(panel);
            if (name)
            {
                if (strcmp(name, "MatSystemTopPanel") == 0)
                    g_mat_system_top_panel = panel;
                else if (strcmp(name, "HudZoom") == 0)
                    g_hud_zoom_panel = panel;
            }
            // After some iterations, assume cached
            static int cache_tries = 0;
            if (++cache_tries > 100)
                g_panels_cached = true;
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    // Fast path using cached IDs
    if (g_panels_cached)
    {
        if (panel == g_hud_zoom_panel && g_variables && g_variables->removals_scope)
            return;
    }
    else
    {
        // Slow path with strcmp
        if (g_interfaces && g_interfaces->get_vgui_panel() && g_variables)
        {
            __try {
                const char* name = g_interfaces->get_vgui_panel()->GetName(panel);
                if (name)
                {
                    if (strcmp("HudZoom", name) == 0 && g_variables->removals_scope)
                        return;
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {}
        }
    }

    __try {
        paint_traverse_original(ecx, panel, force_repaint, allow_force);
    } __except(EXCEPTION_EXECUTE_HANDLER) { return; }

    bool is_mat_system_top = false;
    if (g_panels_cached)
        is_mat_system_top = (panel == g_mat_system_top_panel);
    else if (g_interfaces && g_interfaces->get_vgui_panel())
    {
        __try {
            is_mat_system_top = (strcmp(g_interfaces->get_vgui_panel()->GetName(panel), "MatSystemTopPanel") == 0);
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    if (is_mat_system_top)
    {
        if (g_context && !g_context->initialised_fonts)
        {
            __try {
                if (g_render)
                    g_render->initialize();
                g_context->initialised_fonts = true;
            } __except(EXCEPTION_EXECUTE_HANDLER) {}
        }

        if (g_gui)
        {
            __try { g_gui->unlock_cursor(); } __except(EXCEPTION_EXECUTE_HANDLER) {}
        }

        if (g_visuals)
        {
            __try { g_visuals->draw(); } __except(EXCEPTION_EXECUTE_HANDLER) {}
        }

        if (g_esp)
        {
            __try { g_esp->draw(); } __except(EXCEPTION_EXECUTE_HANDLER) {}
        }
    }
}

void c_hooking::initialize_vgui_panel()
{
    if (!vgui_panel_table)
        return;
    vgui_panel_table->hook_function(reinterpret_cast<uintptr_t>(hk_paint_traverse), 41);
}
