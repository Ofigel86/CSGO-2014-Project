#include "movement.hpp"

c_movement* g_movement = new c_movement;

void c_movement::bunny_hop(CUserCmd* command)
{
	if (!g_variables->movement_bunnyhop)
		return;

	if (!g_context->local_player->is_alive())
		return;

	if (command->m_buttons & IN_JUMP && !(g_context->local_player->m_flags() & FL_ONGROUND))
		command->m_buttons &= ~IN_JUMP;
}

void c_movement::auto_strafe(CUserCmd* command)
{
	if (!g_variables->movement_autostrafe)
		return;

	if (!(g_context->local_player->m_flags() & FL_ONGROUND) && command->m_sidemove == 0.0 && command->m_forwardmove == 0.0) // autostrafer
	{
		static float fOldAbsoluteViewYaw;
		float fAbsoluteView = g_context->old_angle.yaw;

		float fDeltaAbsView = fAbsoluteView - fOldAbsoluteViewYaw;

		fOldAbsoluteViewYaw = fAbsoluteView;

		if (fDeltaAbsView > 0.0f)
			command->m_sidemove = -450.0;
		else if (fDeltaAbsView < 0.0f)
			command->m_sidemove = 450.0;
	}
}

void c_movement::fix_movement(CUserCmd* command)
{
	Vector view_fwd, view_right, view_up, cmd_fwd, cmd_right, cmd_up;
	auto viewangles = command->m_viewangles;
	viewangles.Normalize();

	g_math->angle_vectors(g_context->old_angle, view_fwd, view_right, view_up);
	g_math->angle_vectors(viewangles, cmd_fwd, cmd_right, cmd_up);

	const float v8 = sqrtf((view_fwd.x * view_fwd.x) + (view_fwd.y * view_fwd.y));
	const float v10 = sqrtf((view_right.x * view_right.x) + (view_right.y * view_right.y));
	const float v12 = sqrtf(view_up.z * view_up.z);

	const Vector norm_view_fwd((1.f / v8) * view_fwd.x, (1.f / v8) * view_fwd.y, 0.f);
	const Vector norm_view_right((1.f / v10) * view_right.x, (1.f / v10) * view_right.y, 0.f);
	const Vector norm_view_up(0.f, 0.f, (1.f / v12) * view_up.z);

	const float v14 = sqrtf((cmd_fwd.x * cmd_fwd.x) + (cmd_fwd.y * cmd_fwd.y));
	const float v16 = sqrtf((cmd_right.x * cmd_right.x) + (cmd_right.y * cmd_right.y));
	const float v18 = sqrtf(cmd_up.z * cmd_up.z);

	const Vector norm_cmd_fwd((1.f / v14) * cmd_fwd.x, (1.f / v14) * cmd_fwd.y, 0.f);
	const Vector norm_cmd_right((1.f / v16) * cmd_right.x, (1.f / v16) * cmd_right.y, 0.f);
	const Vector norm_cmd_up(0.f, 0.f, (1.f / v18) * cmd_up.z);

	const float v22 = norm_view_fwd.x * command->m_forwardmove;
	const float v26 = norm_view_fwd.y * command->m_forwardmove;
	const float v28 = norm_view_fwd.z * command->m_forwardmove;
	const float v24 = norm_view_right.x * command->m_sidemove;
	const float v23 = norm_view_right.y * command->m_sidemove;
	const float v25 = norm_view_right.z * command->m_sidemove;
	const float v30 = norm_view_up.x * command->m_upmove;
	const float v27 = norm_view_up.z * command->m_upmove;
	const float v29 = norm_view_up.y * command->m_upmove;

	command->m_forwardmove = ((((norm_cmd_fwd.x * v24) + (norm_cmd_fwd.y * v23)) + (norm_cmd_fwd.z * v25))
		+ (((norm_cmd_fwd.x * v22) + (norm_cmd_fwd.y * v26)) + (norm_cmd_fwd.z * v28)))
		+ (((norm_cmd_fwd.y * v30) + (norm_cmd_fwd.x * v29)) + (norm_cmd_fwd.z * v27));
	command->m_sidemove = ((((norm_cmd_right.x * v24) + (norm_cmd_right.y * v23)) + (norm_cmd_right.z * v25))
		+ (((norm_cmd_right.x * v22) + (norm_cmd_right.y * v26)) + (norm_cmd_right.z * v28)))
		+ (((norm_cmd_right.x * v29) + (norm_cmd_right.y * v30)) + (norm_cmd_right.z * v27));
	command->m_upmove = ((((norm_cmd_up.x * v23) + (norm_cmd_up.y * v24)) + (norm_cmd_up.z * v25))
		+ (((norm_cmd_up.x * v26) + (norm_cmd_up.y * v22)) + (norm_cmd_up.z * v28)))
		+ (((norm_cmd_up.x * v30) + (norm_cmd_up.y * v29)) + (norm_cmd_up.z * v27));

	command->m_forwardmove = g_math->clamp(command->m_forwardmove, -450.f, 450.f);
	command->m_sidemove = g_math->clamp(command->m_sidemove, -450.f, 450.f);
	command->m_upmove = g_math->clamp(command->m_upmove, -320.f, 320.f);
}