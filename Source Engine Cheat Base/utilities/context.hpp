#pragma once
#include "../math/QAngle.hpp"
#include <string>
#include <atomic>

class c_cs_player;
class c_base_combat_weapon;

class c_context
{
public:
    c_cs_player* local_player = nullptr;
    c_base_combat_weapon* local_weapon = nullptr;

    QAngle old_angle{};
    bool initialised_fonts = false;
    std::atomic<bool> initialized = false;

    std::string cheat_user = "dev";
    std::string cheat_name = "project3";
    std::string cheat_version = " [alpha - fixed]";
};

extern c_context* g_context;
