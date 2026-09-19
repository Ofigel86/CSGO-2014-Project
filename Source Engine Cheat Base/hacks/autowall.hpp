#pragma once
#include "../interfaces/interfaces.hpp"

class c_autowall
{
public:
    float get_damage(Vector start, Vector end);
    bool can_hit_point(Vector start, Vector end, c_cs_player* target);
    bool trace_to_exit(Vector& end, trace_t& enter_trace, Vector start, Vector dir, trace_t& exit_trace);
    void clip_trace_to_players(Vector& start, Vector& end, trace_t& trace, CTraceFilter* filter);
};

extern c_autowall* g_autowall;
