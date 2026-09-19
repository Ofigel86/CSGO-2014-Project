#include "esp.hpp"

c_esp* g_esp = new c_esp;

void c_esp::draw()
{
	if (!g_variables->esp_enabled)
		return;

	for (auto i = 0; i < 65; i++)
	{
		c_cs_player* player = reinterpret_cast<c_cs_player*>(g_interfaces->get_client_entity_list()->GetClientEntity(i));

		if (!player->is_alive())
			continue;

		if (player == g_context->local_player)
			continue;

		get_render_bounds(player);

		draw_name(player);
		draw_bounding_box(player);
		draw_health_bar(player);
		draw_flags(player);
	}
}

void c_esp::get_render_bounds(c_cs_player* player)
{
	Vector min, max;
	player->GetRenderBounds(min, max);

	draw_pos_3d = player->GetAbsOrigin() - Vector(0, 0, 10);
	draw_top_3d = draw_pos_3d + Vector(0, 0, max.z + 11);
}

void c_esp::draw_name(c_cs_player* player)
{
	if (!g_variables->esp_name)
		return;

	Vector screen_pos, screen_top;

	player_info_t info;
	g_interfaces->get_engine_client()->GetPlayerInfo(player->EntIndex(), &info);

	float text_width = g_render->get_text_size(g_render->verdana_font, info.name).x / 2.f;

	if (g_render->world_to_screen(draw_pos_3d, screen_pos) && g_render->world_to_screen(draw_top_3d, screen_top) && player->m_team() != g_context->local_player->m_team())
	{
		int height = (screen_pos.y - screen_top.y);
		int width = height / 2;

		int x = screen_pos.x - (width / 2);
		int y = screen_top.y;

		g_render->draw_text(g_render->verdana_font, (x + (width / 2)) - text_width, y - 14, Color(255, 255, 255, 255), 0, info.name);
	}
}

void c_esp::draw_bounding_box(c_cs_player* player)
{
	if (!g_variables->esp_bounding_box)
		return;

	Vector screen_pos, screen_top;

	if (g_render->world_to_screen(draw_pos_3d, screen_pos) && g_render->world_to_screen(draw_top_3d, screen_top) && player->m_team() != g_context->local_player->m_team())
	{
		int height = (screen_pos.y - screen_top.y);
		int width = height / 2;

		int x = screen_pos.x - ((width / 2) / 2);
		int x2 = screen_pos.x - (width / 2);

		int y = screen_top.y;

		int w = width / 2;
		int h = height;

		int iw = w / 3.5;
		int ih = (h / 3.5) - 1;

		g_render->draw_outlined_rect(x2, y, width, height, Color(160, 160, 255));
		g_render->draw_outlined_rect(x2 - 1, y - 1, width + 2, height + 2, Color(0, 0, 0));
		g_render->draw_outlined_rect(x2 + 1, y + 1, width - 2, height - 2, Color(0, 0, 0));
	}
}

void c_esp::draw_health_bar(c_cs_player* player)
{
	if (!g_variables->esp_health_bar)
		return;

	Vector screen_pos, screen_top;

	if (g_render->world_to_screen(draw_pos_3d, screen_pos) && g_render->world_to_screen(draw_top_3d, screen_top) && player->m_team() != g_context->local_player->m_team())
	{
		int height = (screen_pos.y - screen_top.y);
		int width = height / 2;

		int x = screen_pos.x - (width / 2);
		int y = screen_top.y;

		int health = player->m_health();

		auto bar_height = (int)((float)health * (float)height / 100.0f);
		auto offset = height - bar_height;

		g_render->draw_filled_rect(x - 6, y, 4, height, Color(0, 0, 0));
		g_render->draw_filled_rect(x - 6, y + offset, 4, bar_height, Color(65, 245, 35));
		g_render->draw_outlined_rect(x - 6, y, 4, height, Color(0, 0, 0));

		if (health < 100)
			g_render->draw_text(g_render->small_font, x - g_render->get_text_size(g_render->small_font, std::to_string(health).c_str()).x, y + offset - 1, Color(255, 255, 255), 0, std::to_string(health).c_str());
	}
}

void c_esp::draw_flags(c_cs_player* player)
{
	if (!g_variables->esp_flags)
		return;
}