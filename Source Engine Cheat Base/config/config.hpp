#pragma once
#include <string>

class c_config_manager
{
public:
    void Save(std::string path);
    void Load(std::string path);
};

class c_config_variables
{
public:
    bool ragebot_enabled = false;
    bool ragebot_autofire = false;
    bool ragebot_autoscope = false;

    bool legitbot_enabled = false;

    bool antiaim_enabled = false;
    int antiaim_yaw_mode = 4; // 0-none,1-back,2-side,3-180,4-jitter,5-desync,6-spin
    int antiaim_pitch_mode = 1; // 0-none,1-down,2-up,3-zero,4-jitter
    bool antiaim_fakelag_enabled = false;
    int antiaim_fakelag_ticks = 1;
    bool antiaim_lby_breaker = false;
    
    // Jitter settings - configurable radius, default jitters at max speed
    int antiaim_jitter_range = 45; // radius for yaw jitter (e.g., 45 = +-45)
    int antiaim_jitter_range_pitch = 10; // radius for pitch jitter
    bool antiaim_jitter_random = false; // random jitter within range vs fixed
    int antiaim_jitter_speed = 1; // 1 = max speed (every tick), 2 = every 2 ticks, etc.

    bool chams_enabled = false;
    bool chams_enemy_enabled = false;
    int chams_material = 0;
    float chams_enemy_color_visible[4] = { 0.44f, 0.55f, 0.66f, 1.0f };
    float chams_enemy_color_invisible[4] = { 0.66f, 0.55f, 0.44f, 1.0f };

    bool esp_enabled = false;
    bool esp_bounding_box = false;
    bool esp_health_bar = false;
    bool esp_flags = false;
    bool esp_name = false;

    bool visuals_thirdperson = false;
    int visuals_thirdperson_key = 0;
    int visuals_thirdperson_distance = 100;
    int visuals_viewmodel_fov = 0;
    int visuals_world_fov = 0;
    bool visuals_fov_in_scope = false;
    bool visuals_c4_timer = false;

    bool removals_scope = false;
    bool removals_flash = false;
    bool removals_visual_recoil = false;

    bool movement_bunnyhop = false;
    bool movement_autostrafe = false;

    bool misc_nospread = false;
    bool misc_norecoil = false;

    bool misc_watermark = true;
};

class c_keybinds
{
    bool thirdperson = false;
public:
    void handle_toggled_keybinds();
    bool get_thirdperson_state();
};

extern c_config_variables* g_variables;
extern c_config_manager* g_config_manager;
extern c_keybinds* g_keybinds;
