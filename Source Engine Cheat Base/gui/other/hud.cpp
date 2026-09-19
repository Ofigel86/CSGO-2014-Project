#include "../gui.hpp"
#include "../../utilities/context.hpp"
#include "../../game/managers/draw_manager.hpp"

#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>

#pragma warning(disable : 4996)

void c_gui::draw_hud()
{
    if (!g_render)
        return;

    __try {
        draw_watermark();
        draw_keybinds();
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void c_gui::draw_watermark()
{
    if (!g_variables || !g_variables->misc_watermark)
        return;
    if (!g_context || !g_render || !initialized_directx)
        return;

    __try {
        std::string watermark_text;

        auto t = std::time(nullptr);
        std::ostringstream time_stream;
        time_stream << std::put_time(std::localtime(&t), "%H:%M:%S");

        watermark_text = g_context->cheat_name + g_context->cheat_version + " | " + g_context->cheat_user + " | " + time_stream.str() + " | FPS: " + std::to_string(static_cast<int>(ImGui::GetIO().Framerate));

        float text_size = ImGui::CalcTextSize(watermark_text.c_str()).x;
        if (text_size <= 0)
            return;

        ImGui::SetNextWindowPos(ImVec2(static_cast<float>(g_render->screen_width) - (text_size + 30.0f), 10.0f));
        ImGui::SetNextWindowSize(ImVec2(text_size + 27.0f, 38.0f));
        ImGui::Begin("watermark", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        if (draw_list)
        {
            ImVec2 window_pos = ImGui::GetWindowPos();
            draw_list->AddRectFilled(ImVec2(window_pos.x, window_pos.y + 2), ImVec2(window_pos.x + text_size + 20, window_pos.y + 38), ImGui::ColorConvertFloat4ToU32(ImVec4(0.07f, 0.07f, 0.07f, 0.82f)));
            draw_list->AddRectFilled(ImVec2(window_pos.x, window_pos.y), ImVec2(window_pos.x + text_size + 20, window_pos.y + 4), ImGui::ColorConvertFloat4ToU32(ImVec4(0.07f, 0.07f, 0.97f, 0.82f)));
            draw_list->AddText(ImVec2(window_pos.x + 12, window_pos.y + 9), ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)), watermark_text.c_str());
        }

        ImGui::End();
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void c_gui::draw_keybinds()
{
    if (!g_keybinds || !g_variables)
        return;

    // TODO: Draw active keybinds list
}
