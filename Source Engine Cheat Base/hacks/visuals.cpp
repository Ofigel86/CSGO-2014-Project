#include <algorithm>
#include "visuals.hpp"

c_visuals* g_visuals = new c_visuals();

void c_visuals::draw()
{
    if (!g_interfaces || !g_context)
        return;

    draw_scope_lines();

    auto entity_list = g_interfaces->get_client_entity_list();
    if (!entity_list)
        return;

    int highest = 0;
    __try {
        highest = entity_list->GetHighestEntityIndex();
    } __except(EXCEPTION_EXECUTE_HANDLER) { return; }

    highest = std::min(highest, 2048); // sanity limit

    for (auto i = 0; i < highest; i++)
    {
        c_base_entity* entity = nullptr;
        __try {
            entity = reinterpret_cast<c_base_entity*>(entity_list->GetClientEntity(i));
        } __except(EXCEPTION_EXECUTE_HANDLER) { continue; }

        if (!entity)
            continue;

        auto client_class = entity->GetClientClass();
        if (!client_class)
            continue;

        __try {
            switch (client_class->m_ClassID)
            {
            case CPlantedC4:
                {
                    c_c4* bomb = reinterpret_cast<c_c4*>(entity);
                    if (bomb)
                        bomb_timer(bomb);
                }
                break;
            default:
                break;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) { continue; }
    }
}

void c_visuals::bomb_esp(c_c4* bomb)
{
    // TODO: Implement bomb ESP
    if (!bomb || !g_render)
        return;
}

void c_visuals::bomb_timer(c_c4* bomb)
{
    if (!g_variables || !g_variables->visuals_c4_timer)
        return;
    if (!bomb || !g_interfaces || !g_render)
        return;

    auto cvars = g_interfaces->get_cvars();
    auto global_vars = g_interfaces->get_global_vars();
    if (!cvars || !global_vars)
        return;

    ConVar* c4timer_cvar = nullptr;
    __try {
        c4timer_cvar = cvars->FindVar("mp_c4timer");
    } __except(EXCEPTION_EXECUTE_HANDLER) { return; }

    if (!c4timer_cvar)
        return;

    float c4timer_float = 40.0f;
    __try { c4timer_float = c4timer_cvar->GetFloat(); } __except(EXCEPTION_EXECUTE_HANDLER) { return; }
    if (c4timer_float <= 0.0f)
        c4timer_float = 40.0f;

    float time_to_explode = 0.0f;
    __try {
        time_to_explode = bomb->m_c4_blow() - global_vars->m_curtime;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return; }

    if (time_to_explode <= 0.0f)
        return;

    float timer_rect_width = time_to_explode / c4timer_float;
    timer_rect_width = std::max(0.0f, std::min(1.0f, timer_rect_width));

    __try {
        g_render->draw_filled_rect(0, 0, 12, g_render->screen_height, Color(255, 30, 30, 120));
        g_render->draw_filled_rect(0, 0, 12, static_cast<int>(g_render->screen_height * timer_rect_width), Color(40, 255, 40, 120));
        std::string time_str = std::to_string(static_cast<int>(time_to_explode));
        int text_w = static_cast<int>(g_render->get_text_size(g_render->verdana_font, time_str.c_str()).x);
        g_render->draw_text(g_render->verdana_font, 12 - (text_w / 2), static_cast<int>(g_render->screen_height * timer_rect_width) - 6, Color(255, 255, 255, 210), 0, time_str.c_str());
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void c_visuals::grenade_esp(c_base_entity* grenade)
{
    if (!grenade || !g_render)
        return;
    // TODO
}

void c_visuals::grenade_tracers(c_base_entity* grenade)
{
    if (!grenade || !g_render)
        return;
    // TODO
}

void c_visuals::draw_scope_lines()
{
    if (!g_variables || !g_variables->removals_scope)
        return;
    if (!g_context || !g_context->local_player || !g_render)
        return;

    bool is_scoped = false;
    __try { is_scoped = g_context->local_player->m_is_scoped(); } __except(EXCEPTION_EXECUTE_HANDLER) { return; }

    if (!is_scoped)
        return;

    __try {
        g_render->draw_line(g_render->screen_width / 2, 0, g_render->screen_width / 2, g_render->screen_height, Color(0,0,0,255));
        g_render->draw_line(0, g_render->screen_height / 2, g_render->screen_width, g_render->screen_height / 2, Color(0,0,0,255));
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}
