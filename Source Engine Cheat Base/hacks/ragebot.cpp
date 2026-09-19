#include "autowall.hpp"
#include "ragebot.hpp"
#include "lag_comp.hpp"
#include "resolver.hpp"
#include "../config/config.hpp"

c_ragebot* g_ragebot = new c_ragebot();

void c_ragebot::instance(CUserCmd* command)
{
    if (!command)
        return;
    if (!g_variables || !g_variables->ragebot_enabled)
        return;
    if (!g_context || !g_context->local_player || !g_context->local_weapon)
        return;
    if (!g_context->local_player->is_alive())
        return;
    if (g_context->local_weapon->is_non_aim())
        return;

    // Update lag comp and resolver (lag records against jitters)
    if (g_lag_comp)
    {
        __try { g_lag_comp->instance(); } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
    if (g_resolver)
    {
        __try { g_resolver->instance(); } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    target_index = -1;
    target_selection();

    if (target_index == -1 || target_index < 0 || target_index >= 65)
        return;

    auto entity_list = g_interfaces->get_client_entity_list();
    if (!entity_list)
        return;

    c_cs_player* target = reinterpret_cast<c_cs_player*>(entity_list->GetClientEntity(target_index));
    if (!target || !target->is_alive())
        return;

    aim_at_target(command, target);
    fire(command);
}

void c_ragebot::target_selection()
{
    if (!g_context || !g_context->local_player)
        return;

    auto entity_list = g_interfaces->get_client_entity_list();
    if (!entity_list)
        return;

    float lowest = 999999.0f;
    int best_index = -1;

    for (auto i = 1; i < 65; i++)
    {
        c_cs_player* target = reinterpret_cast<c_cs_player*>(entity_list->GetClientEntity(i));
        if (!target)
            continue;
        if (target == g_context->local_player)
            continue;

        __try
        {
            if (!target->is_alive())
                continue;
            if (target->m_team() == g_context->local_player->m_team())
                continue;
            if (target->m_gun_game_immunity())
                continue;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) { continue; }

        float dist = 0.f;
        __try {
            dist = g_context->local_player->m_origin().DistTo(target->m_origin());
        } __except(EXCEPTION_EXECUTE_HANDLER) { continue; }

        Vector aim_point = get_aimbot_point(target);
        if (aim_point.IsZero())
            continue;

        if (!is_point_visible(target, g_context->local_player->get_eye_position(), aim_point))
            continue;

        if (dist < lowest)
        {
            lowest = dist;
            best_index = i;
        }
    }

    target_index = best_index;
}

Vector c_ragebot::get_aimbot_point(c_cs_player* target)
{
    if (!target)
        return Vector(0,0,0);
    // 8 = head, 10 = upper chest? Using head bone for now
    __try {
        return target->get_bone_position(8);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return Vector(0,0,0);
    }
}

QAngle c_ragebot::get_aimbot_angle(Vector src, Vector dst)
{
    QAngle angles;
    Vector delta = src - dst;

    if (g_math)
        g_math->vector_angles(delta, angles);
    else
        angles = QAngle(0,0,0);

    angles.Normalize();
    return angles;
}

void c_ragebot::aim_at_target(CUserCmd* command, c_cs_player* target)
{
    if (!command || !target || !g_context || !g_context->local_player)
        return;

    // Resolver against jitters using lag records
    if (g_variables && g_variables->ragebot_resolver && g_resolver && g_lag_comp)
    {
        __try {
            // Find target index
            int idx = -1;
            auto entity_list = g_interfaces->get_client_entity_list();
            if (entity_list)
            {
                for (int i = 1; i < 65; i++)
                {
                    if (entity_list->GetClientEntity(i) == target)
                    {
                        idx = i;
                        break;
                    }
                }
            }
            
            if (idx != -1)
            {
                auto record = g_lag_comp->get_latest_record(idx);
                if (record && g_resolver->is_jittering_from_history(idx))
                {
                    // Use resolved angle for aimbot point adjustment
                    // For 2014, resolver tries to predict real angle from jitter pattern
                    QAngle resolved = g_resolver->get_resolved_angle(idx);
                    // If resolved valid, we could adjust aim point, but for now just use normal
                    // The resolver already overrides record's eye angles
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    Vector aimbot_point = get_aimbot_point(target);
    if (aimbot_point.IsZero())
        return;

    Vector eye_position = g_context->local_player->get_eye_position();
    if (eye_position.IsZero())
        return;

    command->m_viewangles = get_aimbot_angle(eye_position, aimbot_point);
}

void c_ragebot::fire(CUserCmd* command)
{
    if (!command || !g_variables || !g_context || !g_context->local_weapon || !g_context->local_player)
        return;
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
    if (!command || !g_variables || !g_context || !g_context->local_weapon || !g_context->local_player)
        return;
    if (!g_variables->ragebot_autoscope)
        return;
    if (g_context->local_weapon->is_sniper() && !g_context->local_player->m_is_scoped())
        command->m_buttons |= IN_ZOOM;
}

bool c_ragebot::is_point_visible(c_cs_player* target, Vector source, Vector point)
{
    if (!target || !g_interfaces)
        return false;

    auto trace = g_interfaces->get_engine_trace();
    if (!trace)
        return false;

    trace_t tr;
    CTraceFilterSkipTwoEntities trace_filter;
    trace_filter.pSkip1 = g_context ? g_context->local_player : nullptr;
    trace_filter.pSkip2 = target;

    Ray_t ray;
    ray.Init(source, point);

    __try {
        trace->TraceRay(ray, MASK_SHOT, &trace_filter, &tr);
        return (tr.fraction >= 0.99f || tr.m_pEnt == target);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

float c_ragebot::get_wall_damage(Vector start, Vector end)
{
    // TODO: Proper autowall implementation
    // For now return 0 to avoid undefined behavior
    if (g_autowall)
        return g_autowall->get_damage(start, end);
    return 0.0f;
}

bool c_ragebot::hit_chance(CUserCmd* command)
{
    // TODO: Implement hitchance calculation
    // For now return true to avoid blocking shots
    if (!command || !g_context || !g_context->local_weapon)
        return false;
    return true;
}
