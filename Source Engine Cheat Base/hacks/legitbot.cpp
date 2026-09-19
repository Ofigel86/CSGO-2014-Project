#include "legitbot.hpp"

c_legitbot* g_legitbot = new c_legitbot();

void c_legitbot::instance(CUserCmd* command)
{
    if (!command || !g_variables || !g_context)
        return;
    if (!g_variables->legitbot_enabled)
        return;
    if (!g_context->local_player || !g_context->local_player->is_alive())
        return;
    if (!g_context->local_weapon || g_context->local_weapon->is_non_aim())
        return;

    // TODO: Implement legitbot with smoothing, FOV, RCS
    // For now safe stub that does nothing to avoid crashes
}
