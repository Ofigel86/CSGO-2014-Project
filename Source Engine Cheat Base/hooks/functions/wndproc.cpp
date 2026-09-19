#include "../hooks.hpp"
#include "../../gui/gui.hpp"

#include <thread>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace csgo_window
{
    WNDPROC original_wndproc = nullptr;
    HWND window = nullptr;
}

LRESULT __stdcall hk_wndproc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    if (!g_gui)
        return CallWindowProc(csgo_window::original_wndproc, hWnd, uMsg, wParam, lParam);

    if (uMsg == WM_KEYDOWN && wParam == VK_INSERT)
        g_gui->menu_state = !g_gui->menu_state;

    // Alternative toggle with GetAsyncKeyState removed - use only WM_KEYDOWN to avoid polling

    if (g_gui->menu_state && g_gui->initialized_directx)
    {
        __try {
            if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
                return true;
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    if (g_gui->menu_state)
    {
        // Block game input when menu is open
        if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_MOUSEMOVE || uMsg == WM_MOUSEWHEEL)
            return true;
    }

    __try {
        return CallWindowProc(csgo_window::original_wndproc, hWnd, uMsg, wParam, lParam);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

void c_hooking::initialize_wndproc()
{
    int attempts = 0;
    while (!(csgo_window::window = FindWindowA("Valve001", nullptr)) && attempts < 100)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        attempts++;
    }

    if (!csgo_window::window)
        return;

    __try {
        csgo_window::original_wndproc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(csgo_window::window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(hk_wndproc)));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        csgo_window::original_wndproc = nullptr;
    }
}
