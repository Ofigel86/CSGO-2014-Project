#include <algorithm>
#include "esp.hpp"

c_esp* g_esp = new c_esp();

void c_esp::draw()
{
    if (!g_variables || !g_variables->esp_enabled)
        return;
    if (!g_context || !g_interfaces)
        return;

    auto entity_list = g_interfaces->get_client_entity_list();
    if (!entity_list)
        return;

    auto local = g_context->local_player;
    if (!local)
        return;

    for (auto i = 1; i < 65; i++)
    {
        c_cs_player* player = reinterpret_cast<c_cs_player*>(entity_list->GetClientEntity(i));
        if (!player)
            continue;
        if (player == local)
            continue;

        __try {
            if (!player->is_alive())
                continue;
        } __except(EXCEPTION_EXECUTE_HANDLER) { continue; }

        get_render_bounds(player);

        draw_name(player);
        draw_bounding_box(player);
        draw_health_bar(player);
        draw_flags(player);
    }
}

void c_esp::get_render_bounds(c_cs_player* player)
{
    if (!player)
        return;

    __try {
        Vector min, max;
        player->GetRenderBounds(min, max);
        draw_pos_3d = player->GetAbsOrigin() - Vector(0, 0, 10);
        draw_top_3d = draw_pos_3d + Vector(0, 0, max.z + 11);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        draw_pos_3d = Vector(0,0,0);
        draw_top_3d = Vector(0,0,0);
    }
}

void c_esp::draw_name(c_cs_player* player)
{
    if (!g_variables || !g_variables->esp_name)
        return;
    if (!player || !g_interfaces || !g_render || !g_context || !g_context->local_player)
        return;

    __try {
        if (player->m_team() == g_context->local_player->m_team())
            return;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return; }

    Vector screen_pos, screen_top;
    player_info_t info{};
    
    auto engine = g_interfaces->get_engine_client();
    if (!engine)
        return;

    __try {
        if (!engine->GetPlayerInfo(player->EntIndex(), &info))
            return;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return; }

    if (!info.name || strlen(info.name) == 0)
        return;

    float text_width = 0.f;
    __try {
        text_width = g_render->get_text_size(g_render->verdana_font, info.name).x / 2.f;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return; }

    if (g_render->world_to_screen(draw_pos_3d, screen_pos) && g_render->world_to_screen(draw_top_3d, screen_top))
    {
        int height = static_cast<int>(screen_pos.y - screen_top.y);
        int width = height / 2;

        int x = static_cast<int>(screen_pos.x - (width / 2));
        int y = static_cast<int>(screen_top.y);

        __try {
            g_render->draw_text(g_render->verdana_font, static_cast<int>((x + (width / 2)) - text_width), y - 14, Color(255, 255, 255, 255), 0, info.name);
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
}

void c_esp::draw_bounding_box(c_cs_player* player)
{
    if (!g_variables || !g_variables->esp_bounding_box)
        return;
    if (!player || !g_render || !g_context || !g_context->local_player)
        return;

    __try {
        if (player->m_team() == g_context->local_player->m_team())
            return;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return; }

    Vector screen_pos, screen_top;

    if (g_render->world_to_screen(draw_pos_3d, screen_pos) && g_render->world_to_screen(draw_top_3d, screen_top))
    {
        int height = static_cast<int>(screen_pos.y - screen_top.y);
        int width = height / 2;
        int x2 = static_cast<int>(screen_pos.x - (width / 2));
        int y = static_cast<int>(screen_top.y);

        __try {
            g_render->draw_outlined_rect(x2, y, width, height, Color(160, 160, 255));
            g_render->draw_outlined_rect(x2 - 1, y - 1, width + 2, height + 2, Color(0, 0, 0));
            g_render->draw_outlined_rect(x2 + 1, y + 1, width - 2, height - 2, Color(0, 0, 0));
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
}

void c_esp::draw_health_bar(c_cs_player* player)
{
    if (!g_variables || !g_variables->esp_health_bar)
        return;
    if (!player || !g_render || !g_context || !g_context->local_player)
        return;

    __try {
        if (player->m_team() == g_context->local_player->m_team())
            return;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return; }

    Vector screen_pos, screen_top;

    if (g_render->world_to_screen(draw_pos_3d, screen_pos) && g_render->world_to_screen(draw_top_3d, screen_top))
    {
        int height = static_cast<int>(screen_pos.y - screen_top.y);
        int width = height / 2;
        int x = static_cast<int>(screen_pos.x - (width / 2));
        int y = static_cast<int>(screen_top.y);

        int health = 0;
        __try { health = player->m_health(); } __except(EXCEPTION_EXECUTE_HANDLER) { return; }
        health = std::max(0, std::min(100, health));

        auto bar_height = static_cast<int>(static_cast<float>(health) * static_cast<float>(height) / 100.0f);
        auto offset = height - bar_height;

        __try {
            g_render->draw_filled_rect(x - 6, y, 4, height, Color(0, 0, 0, 150));
            g_render->draw_filled_rect(x - 6, y + offset, 4, bar_height, Color(65, 245, 35, 255));
            g_render->draw_outlined_rect(x - 6, y, 4, height, Color(0, 0, 0, 255));

            if (health < 100 && health > 0)
                g_render->draw_text(g_render->small_font, x - static_cast<int>(g_render->get_text_size(g_render->small_font, std::to_string(health).c_str()).x) - 2, y + offset - 1, Color(255, 255, 255, 255), 0, std::to_string(health).c_str());
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
}

void c_esp::draw_flags(c_cs_player* player)
{
    if (!g_variables || !g_variables->esp_flags)
        return;
    if (!player || !g_render || !g_context || !g_context->local_player)
        return;

    __try {
        if (player->m_team() == g_context->local_player->m_team())
            return;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return; }

    Vector screen_pos, screen_top;
    if (!g_render->world_to_screen(draw_pos_3d, screen_pos) || !g_render->world_to_screen(draw_top_3d, screen_top))
        return;

    int height = static_cast<int>(screen_pos.y - screen_top.y);
    int width = height / 2;
    int x = static_cast<int>(screen_pos.x + (width / 2) + 2);
    int y = static_cast<int>(screen_top.y);

    int offset = 0;
    __try {
        if (player->m_has_helmet() && player->m_armor_value() > 0)
        {
            g_render->draw_text(g_render->small_font, x, y + offset, Color(255, 255, 255, 255), 0, "HK");
            offset += 10;
        }
        else if (player->m_has_helmet())
        {
            g_render->draw_text(g_render->small_font, x, y + offset, Color(255, 255, 255, 255), 0, "H");
            offset += 10;
        }
        else if (player->m_armor_value() > 0)
        {
            g_render->draw_text(g_render->small_font, x, y + offset, Color(255, 255, 255, 255), 0, "K");
            offset += 10;
        }

        if (player->m_has_defuser())
        {
            g_render->draw_text(g_render->small_font, x, y + offset, Color(100, 200, 255, 255), 0, "KIT");
            offset += 10;
        }

        if (player->m_is_scoped())
        {
            g_render->draw_text(g_render->small_font, x, y + offset, Color(100, 200, 255, 255), 0, "ZOOM");
            offset += 10;
        }

        // Money, flashed etc could be added here
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}
