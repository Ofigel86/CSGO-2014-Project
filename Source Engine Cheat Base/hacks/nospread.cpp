#include <algorithm>
#include "nospread.hpp"

c_nospread* g_nospread = new c_nospread();

void c_nospread::instance(CUserCmd* command)
{
    if (!command || !g_context || !g_context->local_weapon)
        return;

    compensate_spread(command);
    compensate_recoil(command);
}

void c_nospread::compensate_spread(CUserCmd* command)
{
    if (!command || !g_variables || !g_context || !g_context->local_weapon || !g_math)
        return;
    if (!g_variables->misc_nospread)
        return;

    __try {
        g_math->random_seed((command->m_random_seed & 0xFF) + 1);

        float flRandomX = g_math->random_float(0.0f, 2.0f * static_cast<float>(M_PI));
        float flRandomInaccuracy = g_math->random_float(0.0f, g_context->local_weapon->get_inaccuracy());
        float flRandomY = g_math->random_float(0.0f, 2.0f * static_cast<float>(M_PI));
        float flRandomSpread = g_math->random_float(0.0f, g_context->local_weapon->get_spread());

        float flSpreadX = static_cast<float>(cos(flRandomX) * flRandomInaccuracy) + static_cast<float>(cos(flRandomY) * flRandomSpread);
        float flSpreadY = static_cast<float>(sin(flRandomX) * flRandomInaccuracy) + static_cast<float>(sin(flRandomY) * flRandomSpread);

        Vector vecDirShooting, vecRight, vecUp;
        g_math->angle_vectors(g_context->old_angle, vecDirShooting, vecRight, vecUp);

        Vector vecDir = vecDirShooting + flSpreadX * vecRight + flSpreadY * vecUp;
        Vector vecView = 8192.0f * vecDir;

        QAngle angSpreadDirection;
        g_math->vector_angles(vecView, angSpreadDirection);

        float f1 = g_context->old_angle.pitch;
        float f2 = vecDirShooting.z;
        float f3 = flSpreadY;

        // Avoid asin domain errors
        float denom = sqrtf(1.0f - (f3 * f3));
        if (denom < 0.001f) denom = 0.001f;
        float ratio = f2 / denom;
        ratio = std::max(-1.0f, std::min(1.0f, ratio));

        command->m_viewangles.pitch += -(f1 + RAD2DEG(asinf(ratio) - atanf(f3)));
        command->m_viewangles.yaw += g_context->old_angle.yaw - angSpreadDirection.yaw + 180.0f;
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void c_nospread::compensate_recoil(CUserCmd* command)
{
    if (!command || !g_variables || !g_context || !g_context->local_player || !g_interfaces)
        return;
    if (!g_variables->misc_norecoil)
        return;

    __try {
        auto cvars = g_interfaces->get_cvars();
        if (!cvars) return;
        ConVar* weapon_recoil_scale = cvars->FindVar("weapon_recoil_scale");
        if (!weapon_recoil_scale) return;
        float scale = weapon_recoil_scale->GetFloat();
        command->m_viewangles -= scale * g_context->local_player->m_aim_punch_angle();
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}
