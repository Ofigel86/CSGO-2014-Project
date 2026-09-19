#include "ragebot.hpp"

c_ragebot* g_ragebot = new c_ragebot;

void c_ragebot::instance(CUserCmd* command)
{
	if (!g_variables->ragebot_enabled)
		return;

	if (!g_context->local_weapon)
		return;

	target_index = -1;

	target_selection();

	if (target_index != -1)
	{
		c_cs_player* target = reinterpret_cast<c_cs_player*>(g_interfaces->get_client_entity_list()->GetClientEntity(target_index));

		if (!target->is_alive())
			return;

		if (g_context->local_weapon->is_non_aim())
			return;

		aim_at_target(command, target);
		fire(command);
	}
}

void c_ragebot::target_selection()
{
	float tmp = 8192.0; float lowest = 999999.0;

	for (auto i = 0; i < 65; i++)
	{
		c_cs_player* target = reinterpret_cast<c_cs_player*>(g_interfaces->get_client_entity_list()->GetClientEntity(i));

		if (target->is_alive() && target->m_team() != g_context->local_player->m_team())
		{
			tmp = g_context->local_player->m_origin().DistTo(target->m_origin());

			if (tmp < lowest && is_point_visible(target, g_context->local_player->get_eye_position(), get_aimbot_point(target)))
			{
				lowest = tmp;
				target_index = i;
			}
		}
	}
}

Vector c_ragebot::get_aimbot_point(c_cs_player* target)
{
	return target->get_bone_position(10);
}

QAngle c_ragebot::get_aimbot_angle(Vector src, Vector dst)
{
	QAngle angles;
	Vector delta = src - dst;

	g_math->vector_angles(delta, angles);

	angles.Normalize();

	return angles;
}

void c_ragebot::aim_at_target(CUserCmd* command, c_cs_player* target)
{
	Vector aimbot_point = get_aimbot_point(target);
	Vector eye_position = g_context->local_player->get_eye_position();

	command->m_viewangles = get_aimbot_angle(eye_position, aimbot_point);
}

void c_ragebot::fire(CUserCmd* command)
{
	if (!g_variables->ragebot_autofire)
		return;

	if (g_context->local_weapon->m_clip() < 1)
		return;

	if (g_context->local_weapon->is_sniper() && !g_context->local_player->m_is_scoped())
	{
		auto_scope(command);
		return;
	}

	command->m_buttons |= IN_ATTACK;
}

void c_ragebot::auto_scope(CUserCmd* command)
{
	if (!g_variables->ragebot_autoscope)
		return;

	if (g_context->local_weapon->is_sniper() && !g_context->local_player->m_is_scoped())
		command->m_buttons |= IN_ZOOM;
}

bool c_ragebot::is_point_visible(c_cs_player* target, Vector source, Vector point)
{
	trace_t trace;

	CTraceFilterSkipTwoEntities trace_filter;
	trace_filter.pSkip1 = g_context->local_player;
	trace_filter.pSkip2 = target;

	Ray_t ray;
	ray.Init(source, point);

	g_interfaces->get_engine_trace()->TraceRay(ray, MASK_SHOT, &trace_filter, &trace);

	return (trace.fraction == 1.0f);
}

float c_ragebot::get_wall_damage(Vector start, Vector end)
{

}

bool c_ragebot::hit_chance(CUserCmd* command)
{

}