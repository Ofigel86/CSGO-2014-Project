#include "../gui.hpp"

bool c_gui::draw_tab_button(const char* name, bool active, bool last)
{
    if (!name)
        return false;

    ImVec4 color = active ? ImVec4(0.2f, 0.4f, 0.9f, 1.0f) : ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, color);

    float size = calculate_tab_button_size();
    if (size <= 0) size = 100.0f;

    bool pressed = ImGui::Button(name, ImVec2(size, 22.0f));

    ImGui::PopStyleColor();

    if (!last)
        ImGui::SameLine(0.0f, 5.0f);
    
    return pressed;
}

bool c_gui::draw_subtab_button(const char* name, bool active, int subtabs_count, bool last)
{
    if (!name)
        return false;

    if (subtabs_count <= 0)
        subtabs_count = 1;

    ImVec4 color = active ? ImVec4(0.3f, 0.5f, 0.95f, 1.0f) : ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, color);

    float size = calculate_subtab_button_size(subtabs_count);
    if (size <= 0) size = 80.0f;

    bool pressed = ImGui::Button(name, ImVec2(size, 20.0f));

    ImGui::PopStyleColor();

    if (!last)
        ImGui::SameLine(0.0f, 5.0f);

    return pressed;
}

bool c_gui::draw_hotkey_button(const char* name, int* key)
{
    if (!name || !key)
        return false;
    return ImGui::Hotkey(name, key, ImVec2(115, 20));
}

float c_gui::calculate_subtab_button_size(int count)
{
    if (count <= 0)
        count = 1;

    float menu_size_without_padding = menu_size.x - (ImGui::GetStyle().WindowPadding.x * 2.f);
    float spacing_between_tabs_size = 5.0f * static_cast<float>(count - 1);

    float total_tabs_size = menu_size_without_padding - spacing_between_tabs_size;
    if (total_tabs_size <= 0)
        return 80.0f;

    return total_tabs_size / static_cast<float>(count);
}

float c_gui::calculate_tab_button_size()
{
    if (tabs_count <= 0)
        tabs_count = 1;

    float menu_size_without_padding = menu_size.x - (ImGui::GetStyle().WindowPadding.x * 2.f);
    float spacing_between_tabs_size = 5.0f * static_cast<float>(tabs_count - 1);

    float total_tabs_size = menu_size_without_padding - spacing_between_tabs_size;
    if (total_tabs_size <= 0)
        return 100.0f;

    return total_tabs_size / static_cast<float>(tabs_count);
}
