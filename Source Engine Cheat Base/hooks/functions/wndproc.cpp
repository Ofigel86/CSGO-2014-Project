#include "../hooks.hpp"
#include "../../gui/gui.hpp"

#include <thread>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace csgo_window
{
	WNDPROC original_wndproc;
	HWND window = NULL;
}

LRESULT __stdcall hk_wndproc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
		g_gui->menu_state = !g_gui->menu_state;

	if (g_gui->menu_state && g_gui->initialized_directx && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	if (g_gui->menu_state && (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_MOUSEMOVE))
		return false;

	return CallWindowProc(csgo_window::original_wndproc, hWnd, uMsg, wParam, lParam);
}

void c_hooking::initialize_wndproc()
{
	while (!(csgo_window::window = FindWindow("Valve001", nullptr)))
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	csgo_window::original_wndproc = (WNDPROC)SetWindowLongPtr(csgo_window::window, GWL_WNDPROC, (LONG_PTR)hk_wndproc);
}