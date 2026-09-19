#include "../gui.hpp"
#include "../../config/config.hpp"

const char* materials[] =
{
    "Regular",
    "Metallic",
    "Flat"
};

void c_gui::draw_rage_tab()
{
    if (draw_subtab_button("Aimbot", (current_subtab_rage == 0), 2, false)) current_subtab_rage = 0;
    if (draw_subtab_button("Anti aimbot", (current_subtab_rage == 1), 2, true)) current_subtab_rage = 1;

    if (current_subtab_rage == 0)
    {
        if (g_variables)
        {
            ImGui::Checkbox("Enable ragebot", &g_variables->ragebot_enabled);
            ImGui::Checkbox("Auto fire", &g_variables->ragebot_autofire);
            ImGui::Checkbox("Auto scope", &g_variables->ragebot_autoscope);
            
            ImGui::Separator();
            ImGui::Text("Resolver vs Jitters (Lag Records)");
            ImGui::Checkbox("Enable Resolver", &g_variables->ragebot_resolver);
            const char* resolver_modes[] = { "Jitter Detect (avg)", "Bruteforce (4 angles)", "Velocity (moving)" };
            ImGui::Combo("Resolver Mode", &g_variables->ragebot_resolver_mode, resolver_modes, IM_ARRAYSIZE(resolver_modes));
            ImGui::Text("Uses lag records to detect jitter pattern");
            ImGui::Text("Fixed jitter: avg of +range/-range = real");
            ImGui::Text("Fixed: avg center, Random: avg+velocity (no brute)");
        }
    }

    if (current_subtab_rage == 1)
    {
        if (g_variables)
        {
            ImGui::Checkbox("Enable antiaim", &g_variables->antiaim_enabled);
            
            const char* yaw_modes[] = { "None", "Backwards", "Sideways", "180", "Jitter (config radius)", "Desync (config radius)", "Spin (config radius)" };
            const char* pitch_modes[] = { "None", "Down", "Up", "Zero", "Jitter (config radius)" };
            
            ImGui::Combo("Yaw mode", &g_variables->antiaim_yaw_mode, yaw_modes, IM_ARRAYSIZE(yaw_modes));
            ImGui::Combo("Pitch mode", &g_variables->antiaim_pitch_mode, pitch_modes, IM_ARRAYSIZE(pitch_modes));
            
            ImGui::Separator();
            ImGui::Text("Jitter Settings - Configurable Radius, Max Speed");
            ImGui::SliderInt("Jitter Yaw Radius (max 90 in 2014)", &g_variables->antiaim_jitter_range, 0, 90);
            ImGui::SliderInt("Jitter Pitch Radius", &g_variables->antiaim_jitter_range_pitch, 0, 89);
            ImGui::Checkbox("Random Jitter (within radius)", &g_variables->antiaim_jitter_random);
            ImGui::SliderInt("Jitter Speed (1=max speed every tick)", &g_variables->antiaim_jitter_speed, 1, 10);
            ImGui::Text("Max speed = jitter every tick, default jitters");
            ImGui::Text("2014 LIMIT: fake max 90 deg total, >90 real moves!");
            ImGui::Text("For jitter +-range, max range=45 (diff 90)");
            
            ImGui::Separator();
            ImGui::Text("Packet Manager (2014, no LBY)");
            ImGui::Checkbox("Fake lag", &g_variables->antiaim_fakelag_enabled);
            ImGui::SliderInt("Fake lag ticks", &g_variables->antiaim_fakelag_ticks, 1, 14);
            
            ImGui::Separator();
            ImGui::Text("Info: 2014 fake = simple bSendPacket choke");
            ImGui::Text("Real = server sees, Fake = enemies see");
            ImGui::Text("No LBY in 2014 build");
            ImGui::Text("MAX DESYNC 2014 = 90 deg, >90 real uedet");
        }
    }
}

void c_gui::draw_legit_tab()
{
    if (g_variables)
        ImGui::Checkbox("Enable legitbot (WIP)", &g_variables->legitbot_enabled);
    ImGui::Text("Legitbot is under construction");
}

void c_gui::draw_esp_tab()
{
    if (draw_subtab_button("ESP", (current_subtab_visuals == 0), 4, false)) current_subtab_visuals = 0;
    if (draw_subtab_button("Other", (current_subtab_visuals == 1), 4, false)) current_subtab_visuals = 1;
    if (draw_subtab_button("Removals", (current_subtab_visuals == 2), 4, false)) current_subtab_visuals = 2;
    if (draw_subtab_button("Chams", (current_subtab_visuals == 3), 4, true)) current_subtab_visuals = 3;

    if (!g_variables)
        return;

    if (current_subtab_visuals == 0)
    {
        ImGui::Checkbox("Enable esp", &g_variables->esp_enabled);
        ImGui::Checkbox("Name", &g_variables->esp_name);
        ImGui::Checkbox("Bounding box", &g_variables->esp_bounding_box);
        ImGui::Checkbox("Health bar", &g_variables->esp_health_bar);
        ImGui::Checkbox("Flags", &g_variables->esp_flags);
    }

    if (current_subtab_visuals == 1)
    {
        ImGui::Checkbox("Thirdperson", &g_variables->visuals_thirdperson);
        ImGui::SliderInt("Thirdperson distance", &g_variables->visuals_thirdperson_distance, 0, 300);
        ImGui::Hotkey("Thirdperson key", &g_variables->visuals_thirdperson_key, ImVec2(200, 20));
        ImGui::SliderInt("World FOV", &g_variables->visuals_world_fov, 0, 60);
        ImGui::Checkbox("Force FOV in scope", &g_variables->visuals_fov_in_scope);
        ImGui::SliderInt("Viewmodel FOV", &g_variables->visuals_viewmodel_fov, 0, 60);
        ImGui::Checkbox("C4 timer", &g_variables->visuals_c4_timer);
    }

    if (current_subtab_visuals == 2)
    {
        ImGui::Checkbox("Remove scope", &g_variables->removals_scope);
        ImGui::Checkbox("Remove flash", &g_variables->removals_flash);
        ImGui::Checkbox("Remove visual recoil", &g_variables->removals_visual_recoil);
    }

    if (current_subtab_visuals == 3)
    {
        ImGui::Checkbox("Enable chams", &g_variables->chams_enabled);
        ImGui::Checkbox("Enemy chams", &g_variables->chams_enemy_enabled);
        ImGui::Combo("Enemy material", &g_variables->chams_material, materials, IM_ARRAYSIZE(materials));
        ImGui::ColorEdit4("Visible color", g_variables->chams_enemy_color_visible);
        ImGui::ColorEdit4("Invisible color", g_variables->chams_enemy_color_invisible);
    }
}

void c_gui::draw_misc_tab()
{
    if (draw_subtab_button("General", (current_subtab_misc == 0), 2, false)) current_subtab_misc = 0;
    if (draw_subtab_button("Other", (current_subtab_misc == 1), 2, true)) current_subtab_misc = 1;

    if (!g_variables)
        return;

    if (current_subtab_misc == 0)
    {
        ImGui::Checkbox("Bunny hop", &g_variables->movement_bunnyhop);
        ImGui::SameLine(); ImGui::SetCursorPosX(320); 
        ImGui::Checkbox("No spread", &g_variables->misc_nospread);

        ImGui::Checkbox("Auto strafe", &g_variables->movement_autostrafe);
        ImGui::SameLine(); ImGui::SetCursorPosX(320);
        ImGui::Checkbox("No recoil", &g_variables->misc_norecoil);
    }

    if (current_subtab_misc == 1)
    {
        ImGui::Checkbox("Watermark", &g_variables->misc_watermark);
        if (g_config_manager)
        {
            if (ImGui::Button("Save config"))
                g_config_manager->Save("C:\\project3_config.json");
            ImGui::SameLine();
            if (ImGui::Button("Load config"))
                g_config_manager->Load("C:\\project3_config.json");
        }
    }
}

void c_gui::draw_skins_tab()
{
    ImGui::Text("Soon...");
}
