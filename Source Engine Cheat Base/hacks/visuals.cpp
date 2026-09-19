#include "visuals.hpp"

c_visuals* g_visuals = new c_visuals;

void c_visuals::draw()
{
	draw_scope_lines();

	for (auto i = 0; i < g_interfaces->get_client_entity_list()->GetHighestEntityIndex(); i++)
	{
		c_base_entity* entity = reinterpret_cast<c_base_entity*>(g_interfaces->get_client_entity_list()->GetClientEntity(i));

		if (!entity)
			continue;

		auto client_class = entity->GetClientClass();

		if (!client_class)
			continue;

		switch (client_class->m_ClassID)
		{
		case CPlantedC4:
			c_c4* bomb = reinterpret_cast<c_c4*>(entity);

			bomb_timer(bomb);
		}
	}
}

void c_visuals::bomb_esp(c_c4* bomb)
{

}

void c_visuals::bomb_timer(c_c4* bomb)
{
	if (!g_variables->visuals_c4_timer)
		return;

	ConVar* c4timer_cvar = g_interfaces->get_cvars()->FindVar("mp_c4timer");
	float c4timer_float = c4timer_cvar->GetFloat();

	float time_to_explode = bomb->m_c4_blow() - g_interfaces->get_global_vars()->m_curtime;
	float timer_rect_width = time_to_explode / c4timer_float;

	if (time_to_explode > 0.0)
	{
		g_render->draw_filled_rect(0, 0, 12, g_render->screen_height, Color(255, 30, 30, 120));
		g_render->draw_filled_rect(0, 0, 12, (g_render->screen_height * timer_rect_width), Color(40, 255, 40, 120));
		g_render->draw_text(g_render->verdana_font, 12 - (g_render->get_text_size(g_render->verdana_font, std::to_string(int(time_to_explode)).c_str()).x / 2), (g_render->screen_height * timer_rect_width) - 6, Color(255, 255, 255, 210), 0, std::to_string(int(time_to_explode)).c_str());
	}
}

void c_visuals::grenade_esp(c_base_entity* grenade)
{

}

void c_visuals::grenade_tracers(c_base_entity* grenade)
{

}

void c_visuals::draw_scope_lines()
{
	if (!g_variables->removals_scope)
		return;

	if (!g_context->local_player || !g_context->local_player->m_is_scoped())
		return;

	g_render->draw_line(g_render->screen_width / 2, 0, g_render->screen_width / 2, g_render->screen_height, Color::Black);
	g_render->draw_line(0, g_render->screen_height / 2, g_render->screen_width, g_render->screen_height / 2, Color::Black);
}