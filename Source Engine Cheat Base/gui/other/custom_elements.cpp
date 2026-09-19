#include "../gui.hpp"

bool c_gui::draw_tab_button(const char* name, bool active, bool last)
{
    bool pressed = ImGui::Button(name, active, ImVec2(calculate_tab_button_size(), 20));

    if (!last)
        ImGui::SameLine(0.0f, 5.0f);
    
    return pressed;
}

bool c_gui::draw_subtab_button(const char* name, bool active, int subtabs_count, bool last)
{
    bool pressed = ImGui::Button(name, active, ImVec2(calculate_subtab_button_size(subtabs_count), 20));

    if (!last)
        ImGui::SameLine(0.0f, 5.0f);

    return pressed;
}

bool c_gui::draw_hotkey_button(const char* name, int* key)
{
    return ImGui::Hotkey(name, key, ImVec2(115, 20));
}

float c_gui::calculate_subtab_button_size(int count)
{
    float menu_size_without_padding = menu_size.x - (ImGui::GetStyle().WindowPadding.x * 2.f);
    float spacing_between_tabs_size = 5 * (count - 1);

    float total_tabs_size = menu_size_without_padding - spacing_between_tabs_size;

    return total_tabs_size / count;
}

float c_gui::calculate_tab_button_size()
{
    float menu_size_without_padding = menu_size.x - (ImGui::GetStyle().WindowPadding.x * 2.f);
    float spacing_between_tabs_size = 5 * (tabs_count - 1);

    float total_tabs_size = menu_size_without_padding - spacing_between_tabs_size;

    return total_tabs_size / tabs_count;
}