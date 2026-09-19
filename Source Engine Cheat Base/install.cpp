#include "win_includes.hpp"
#include "hooks/hooks.hpp"

DWORD WINAPI install_thread(PVOID base)
{
	AllocConsole();

	FILE* str;
	freopen_s(&str, "CONOUT$", "w", stdout);

	g_interfaces->initialize();
	g_interfaces->initialize_netvars();
	g_render->initialize();
	g_hooking_manager->initialize_all_hooks();

	return EXIT_SUCCESS;
}

BOOL WINAPI DllMain(HMODULE module, DWORD call_reason, PVOID reserve)
{
	if (call_reason == DLL_PROCESS_ATTACH)
		CreateThread(nullptr, 0, install_thread, module, 0, nullptr);

	return TRUE;
}