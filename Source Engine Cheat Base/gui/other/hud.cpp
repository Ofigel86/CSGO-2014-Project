#include "../gui.hpp"
#include "../../utilities/context.hpp"
#include "../../game/managers/draw_manager.hpp"

#include <string>
#include <time.h>
#include <iomanip>
#include <ostream>
#include <sstream>

#pragma warning(disable : 4996)

void c_gui::draw_hud()
{
	draw_watermark();
	draw_keybinds();
}

void c_gui::draw_watermark()
{
	if (!g_variables->misc_watermark)
		return;

	std::string watermark_text;

	auto t = std::time(nullptr);
	std::ostringstream time;
	time << std::put_time(std::localtime(&t), "%H:%M:%S");

	watermark_text = g_context->cheat_name + g_context->cheat_version + " | " + g_context->cheat_user + " | " + time.str();

	ImGui::SetNextWindowPos(ImVec2(g_render->screen_width - (ImGui::CalcTextSize(watermark_text.c_str()).x + 30), 10));
	ImGui::SetNextWindowSize(ImVec2(ImGui::CalcTextSize(watermark_text.c_str()).x + 27, 38));
	ImGui::Begin("watermark", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_::ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_::ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_::ImGuiWindowFlags_NoNav);
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		ImVec2 window_pos = ImGui::GetWindowPos();
		ImVec2 window_size = ImGui::GetWindowSize();

		draw_list->AddRectFilled(ImVec2(window_pos.x, window_pos.y + 2), ImVec2(window_pos.x + ImGui::CalcTextSize(watermark_text.c_str()).x + 20, 38), ImGui::ColorConvertFloat4ToU32(ImVec4(0.07, 0.07, 0.07, 0.82)));
		draw_list->AddRectFilled(ImVec2(window_pos.x, window_pos.y), ImVec2(window_pos.x + ImGui::CalcTextSize(watermark_text.c_str()).x + 20, window_pos.y + 4), ImGui::ColorConvertFloat4ToU32(ImVec4(0.07, 0.07, 0.97, 0.82)));
		draw_list->AddText(ImVec2(window_pos.x + 12, window_pos.y + 9), ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)), watermark_text.c_str());
	}
	ImGui::End();
}

void c_gui::draw_keybinds()
{

}