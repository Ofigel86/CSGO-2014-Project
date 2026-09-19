#include "world.hpp"

c_world* g_world = new c_world();

void c_world::player_dlights()
{
    if (!g_interfaces || !g_context || !g_context->local_player)
        return;
    // TODO: Implement dlights for enemies
}

void c_world::player_glow()
{
    if (!g_interfaces || !g_context || !g_context->local_player)
        return;
    // TODO: Implement glow
}
