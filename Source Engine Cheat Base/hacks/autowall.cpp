#include "autowall.hpp"

c_autowall* g_autowall = new c_autowall();

float c_autowall::get_damage(Vector start, Vector end)
{
    if (!g_interfaces || !g_context || !g_context->local_player)
        return 0.0f;

    auto trace = g_interfaces->get_engine_trace();
    if (!trace)
        return 0.0f;

    trace_t tr;
    CTraceFilter filter;
    filter.pSkip = g_context->local_player;

    Ray_t ray;
    ray.Init(start, end);

    __try {
        trace->TraceRay(ray, MASK_SHOT, &filter, &tr);
        if (tr.fraction == 1.0f)
            return 100.0f; // visible, full damage placeholder
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    return 0.0f;
}

bool c_autowall::can_hit_point(Vector start, Vector end, c_cs_player* target)
{
    if (!target || !g_interfaces)
        return false;
    return get_damage(start, end) > 0.0f;
}

bool c_autowall::trace_to_exit(Vector& end, trace_t& enter_trace, Vector start, Vector dir, trace_t& exit_trace)
{
    // Simplified stub - full implementation requires handling breakable surfaces
    return false;
}

void c_autowall::clip_trace_to_players(Vector& start, Vector& end, trace_t& trace, CTraceFilter* filter)
{
    // Stub for now
}
