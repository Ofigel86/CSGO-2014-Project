#include "view.hpp"

c_view* g_view = new c_view;

void c_view::instance(CViewSetup* viewsetup)
{
	if (!g_interfaces->get_engine_client()->IsInGame())
		return;

	world_fov_changer(viewsetup);
	third_person();
}

void c_view::world_fov_changer(CViewSetup* viewsetup)
{
	viewsetup->fov += g_variables->visuals_world_fov;

	if (g_context->local_player->is_alive() && g_context->local_player->m_is_scoped() && g_variables->visuals_fov_in_scope)
		viewsetup->fov = 90.0;
}

void c_view::third_person()
{
	if (g_variables->visuals_thirdperson_distance > 0 && g_variables->visuals_thirdperson && g_keybinds->get_thirdperson_state() && g_context->local_player->is_alive())
	{
		float range = g_variables->visuals_thirdperson_distance;

		if (g_interfaces->get_input()->m_fCameraInThirdPerson)
		{
			g_interfaces->get_engine_client()->ClientCmd_Unrestricted("cl_updatevisibility");
			return;
		}

		QAngle angles;
		g_interfaces->get_engine_client()->GetViewAngles(angles);
		angles.roll = 0.f;

		Vector forward;
		g_math->angle_vectors(angles, forward);

		Vector eye_pos = g_context->local_player->get_eye_position();

		Vector max = Vector(16.f, 16.f, 16.f);

		Ray_t ray;
		ray.Init(eye_pos, eye_pos - forward * range, -max, max);

		CTraceFilter filter;
		filter.pSkip = g_context->local_player;

		trace_t trace;

		g_interfaces->get_engine_trace()->TraceRay(ray, MASK_SOLID, &filter, &trace);

		if (trace.fraction < 1.f)
			range *= trace.fraction;

		angles.roll = range;
		g_interfaces->get_input()->m_fCameraInThirdPerson = true;
		g_interfaces->get_input()->m_vecCameraOffset = angles;

		g_interfaces->get_engine_client()->ClientCmd_Unrestricted("cl_updatevisibility");
	}
	else
	{
		g_interfaces->get_input()->m_fCameraInThirdPerson = false;
		g_interfaces->get_input()->m_vecCameraOffset.roll = 0;
	}
}