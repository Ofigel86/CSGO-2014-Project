#include "gui.hpp"
#include "../hooks/hooks.hpp"

c_gui* g_gui = new c_gui();

void c_gui::initialize(PDIRECT3DDEVICE9 device)
{
    if (initialized_directx || !device)
        return;

    __try {
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

        // Try multiple font paths with fallback
        bool font_loaded = false;
        const char* font_paths[] = {
            "C:\\Windows\\Fonts\\Calibri.ttf",
            "C:\\Windows\\Fonts\\Verdana.ttf",
            "C:\\Windows\\Fonts\\Arial.ttf"
        };

        for (auto path : font_paths)
        {
            if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES)
            {
                io.Fonts->AddFontFromFileTTF(path, 14.0f);
                font_loaded = true;
                break;
            }
        }

        if (!font_loaded)
        {
            // Use default font if no file found
            io.Fonts->AddFontDefault();
        }

        if (csgo_window::window)
            ImGui_ImplWin32_Init(csgo_window::window);
        ImGui_ImplDX9_Init(device);

        menu_size = ImVec2(620, 485);
        initialized_directx = true;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        initialized_directx = false;
    }
}

void c_gui::setup_gui_style()
{
    __try {
        ImGui::StyleColorsDark();
        auto& style = ImGui::GetStyle();
        style.WindowRounding = 4.0f;
        style.FrameRounding = 2.0f;
        style.ScrollbarRounding = 2.0f;
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void c_gui::begin_draw_frame()
{
    if (!initialized_directx)
        return;

    __try {
        if (!menu_state)
            ImGui::GetIO().MouseDrawCursor = false;

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void c_gui::end_draw_frame()
{
    if (!initialized_directx)
        return;

    __try {
        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
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
    __try {
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
        default:
            if (current_tab < 0)
                current_tab = 0;
            break;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void c_gui::unlock_cursor()
{
    if (!g_interfaces || !g_interfaces->get_engine_client())
        return;

    __try {
        // Unlock cursor when menu is open
        if (menu_state)
        {
            // Using VGUI surface to unlock cursor
            auto surface = g_interfaces->get_vgui_surface();
            if (surface)
            {
                surface->UnlockCursor();
            }
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void c_gui::draw_menu()
{
    if (!menu_state || !initialized_directx)
        return;

    __try {
        ImGui::GetStyle().WindowRounding = 4.0f;
        ImGui::GetStyle().TabRounding = 0;
        ImGui::GetStyle().WindowPadding = ImVec2(10, 9);

        ImGui::GetIO().MouseDrawCursor = true;

        ImGui::SetNextWindowSize(menu_size, ImGuiCond_FirstUseEver);
        ImGui::Begin("project3 [fixed]", &menu_state, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
        {
            draw_background();
            draw_tabs_bar();
            draw_content();
        }
        ImGui::End();
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}
