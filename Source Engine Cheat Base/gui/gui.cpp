#include "gui.hpp"
#include "../hooks/hooks.hpp"

c_gui* g_gui = new c_gui;

void c_gui::initialize(PDIRECT3DDEVICE9 device)
{
    if (initialized_directx)
        return;

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Calibri.ttf", 14);

    ImGui_ImplWin32_Init(csgo_window::window);
    ImGui_ImplDX9_Init(device);

    menu_size = ImVec2(620, 485);

    initialized_directx = true;
}

void c_gui::setup_gui_style()
{
    ImGui::StyleColorsDark();
}

void c_gui::begin_draw_frame()
{
    if (!menu_state)
        ImGui::GetIO().MouseDrawCursor = false;

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();
}

void c_gui::end_draw_frame()
{
    ImGui::EndFrame();

    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void c_gui::draw_background()
{
    return;
}

void c_gui::draw_tabs_bar()
{
    tabs_count = 5;

    if (draw_tab_button("Rage", (current_tab == 0), false)) current_tab = 0;
    if (draw_tab_button("Legit", (current_tab == 1), false)) current_tab = 1;
    if (draw_tab_button("Visuals", (current_tab == 2), false)) current_tab = 2;
    if (draw_tab_button("Misc", (current_tab == 3), false)) current_tab = 3;
    if (draw_tab_button("Skins", (current_tab == 4), true)) current_tab = 4;
}

void c_gui::draw_content()
{
    switch (current_tab)
    {
    case 0:
        draw_rage_tab();
        break;
    case 1:
        draw_legit_tab();
        break;
    case 2:
        draw_esp_tab();
        break;
    case 3:
        draw_misc_tab();
        break;
    case 4:
        draw_skins_tab();
        break;
    }
}

void c_gui::unlock_cursor()
{
    
}

void c_gui::draw_menu()
{
    if (!menu_state)
        return;

    ImGui::GetStyle().WindowRounding = 0;
    ImGui::GetStyle().TabRounding = 0;
    ImGui::GetStyle().WindowPadding = ImVec2(10, 9);

    ImGui::GetIO().MouseDrawCursor = true;

    ImGui::SetNextWindowSize(ImVec2(menu_size));
    ImGui::Begin("project3", NULL, ImGuiWindowFlags_NoResize);
    {
        draw_background();
        draw_tabs_bar();
        draw_content();
    }
    ImGui::End();
}