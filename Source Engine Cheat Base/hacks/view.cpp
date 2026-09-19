#include "view.hpp"

c_view* g_view = new c_view();

void c_view::instance(CViewSetup* viewsetup)
{
    if (!viewsetup || !g_interfaces)
        return;

    auto engine = g_interfaces->get_engine_client();
    if (!engine)
        return;

    __try {
        if (!engine->IsInGame())
            return;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return; }

    world_fov_changer(viewsetup);
    third_person();
}

void c_view::world_fov_changer(CViewSetup* viewsetup)
{
    if (!viewsetup || !g_variables || !g_context || !g_context->local_player)
        return;

    __try {
        viewsetup->fov += static_cast<float>(g_variables->visuals_world_fov);

        if (g_context->local_player->is_alive() && g_context->local_player->m_is_scoped() && g_variables->visuals_fov_in_scope)
            viewsetup->fov = 90.0f;
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void c_view::third_person()
{
    if (!g_variables || !g_interfaces || !g_context || !g_context->local_player || !g_math)
        return;

    if (g_variables->visuals_thirdperson_distance <= 0 || !g_variables->visuals_thirdperson || !g_keybinds)
        goto disable_tp;

    __try {
        if (!g_keybinds->get_thirdperson_state() || !g_context->local_player->is_alive())
            goto disable_tp;
    } __except(EXCEPTION_EXECUTE_HANDLER) { goto disable_tp; }

    {
        auto input = g_interfaces->get_input();
        auto engine = g_interfaces->get_engine_client();
        auto trace = g_interfaces->get_engine_trace();
        if (!input || !engine || !trace)
            goto disable_tp;

        float range = static_cast<float>(g_variables->visuals_thirdperson_distance);
        if (range < 10.0f) range = 10.0f;

        if (input->m_fCameraInThirdPerson)
        {
            __try { engine->ClientCmd_Unrestricted("cl_updatevisibility"); } __except(EXCEPTION_EXECUTE_HANDLER) {}
            return;
        }

        QAngle angles{};
        __try { engine->GetViewAngles(angles); } __except(EXCEPTION_EXECUTE_HANDLER) { return; }
        angles.roll = 0.f;

        Vector forward;
        g_math->angle_vectors(angles, forward);

        Vector eye_pos = g_context->local_player->get_eye_position();
        if (eye_pos.IsZero())
            return;

        Vector max = Vector(16.f, 16.f, 16.f);

        Ray_t ray;
        ray.Init(eye_pos, eye_pos - forward * range, -max, max);

        CTraceFilter filter;
        filter.pSkip = g_context->local_player;

        trace_t tr;

        __try {
            trace->TraceRay(ray, MASK_SOLID, &filter, &tr);
            if (tr.fraction < 1.f)
                range *= tr.fraction;
        } __except(EXCEPTION_EXECUTE_HANDLER) {}

        angles.roll = range;
        input->m_fCameraInThirdPerson = true;
        input->m_vecCameraOffset = angles;

        __try { engine->ClientCmd_Unrestricted("cl_updatevisibility"); } __except(EXCEPTION_EXECUTE_HANDLER) {}
        return;
    }

disable_tp:
    {
        auto input = g_interfaces->get_input();
        if (!input)
            return;
        __try {
            input->m_fCameraInThirdPerson = false;
            input->m_vecCameraOffset.roll = 0.f;
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
}
