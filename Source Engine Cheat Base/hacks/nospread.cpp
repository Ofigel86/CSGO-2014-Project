#include "nospread.hpp"

c_nospread* g_nospread = new c_nospread;

void c_nospread::instance(CUserCmd* command)
{
	if (!g_context->local_weapon)
		return;

	compensate_spread(command);
	compensate_recoil(command);
}

void c_nospread::compensate_spread(CUserCmd* command)
{
	if (!g_variables->misc_nospread)
		return;

	g_math->random_seed((command->m_random_seed & 0xFF) + 1);

	float flRandomX = g_math->random_float(0.0f, 2.0f * M_PI);
	float flRandomInaccuracy = g_math->random_float(0.0f, g_context->local_weapon->get_inaccuracy());
	float flRandomY = g_math->random_float(0.0f, 2.0f * M_PI);
	float flRandomSpread = g_math->random_float(0.0f, g_context->local_weapon->get_spread());

	float flSpreadX = float(cos(flRandomX) * flRandomInaccuracy) + (cos(flRandomY) * flRandomSpread);
	float flSpreadY = float(sin(flRandomX) * flRandomInaccuracy) + (sin(flRandomY) * flRandomSpread);
	float flSpreadZ = float((flSpreadX * flSpreadX) + (flSpreadY * flSpreadY));

	Vector vecDirShooting, vecRight, vecUp;
	g_math->angle_vectors(g_context->old_angle, vecDirShooting, vecRight, vecUp);

	Vector vecDir = vecDirShooting + flSpreadX * vecRight + flSpreadY * vecUp;
	Vector vecView = 8192.0f * vecDir;

	QAngle angSpreadDirection;
	g_math->vector_angles(vecView, angSpreadDirection);

	command->m_viewangles.pitch += -(g_context->old_angle.pitch + RAD2DEG(asin(vecDirShooting.z / sqrt(1 - (flSpreadY * flSpreadY))) - atan(flSpreadY)));
	command->m_viewangles.yaw += g_context->old_angle.yaw - angSpreadDirection.yaw + 180.0;
}

void c_nospread::compensate_recoil(CUserCmd* command)
{
	if (!g_variables->misc_norecoil)
		return;

	ConVar* weapon_recoil_scale = g_interfaces->get_cvars()->FindVar("weapon_recoil_scale");
	command->m_viewangles -= weapon_recoil_scale->GetFloat() * g_context->local_player->m_aim_punch_angle();
}