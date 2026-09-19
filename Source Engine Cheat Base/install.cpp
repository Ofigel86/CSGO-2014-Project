#include "win_includes.hpp"
#include "hooks/hooks.hpp"

static HMODULE g_module = nullptr;
static std::atomic<bool> g_unloading = false;

DWORD WINAPI install_thread(PVOID base)
{
    g_module = static_cast<HMODULE>(base);

    // Avoid loader lock issues
    DisableThreadLibraryCalls(g_module);

#ifdef _DEBUG
    AllocConsole();
    FILE* file_out = nullptr;
    FILE* file_err = nullptr;
    freopen_s(&file_out, "CONOUT$", "w", stdout);
    freopen_s(&file_err, "CONOUT$", "w", stderr);
#endif

    // Small delay to let game modules load
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (g_unloading.load())
        return EXIT_SUCCESS;

    __try
    {
        g_interfaces->initialize();
        g_interfaces->initialize_netvars();
        g_render->initialize();
        g_hooking_manager->initialize_all_hooks();
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
#ifdef _DEBUG
        std::cout << "[project3] Failed to initialize, exception caught" << std::endl;
#endif
    }

    return EXIT_SUCCESS;
}

void WINAPI uninstall_thread()
{
    g_unloading.store(true);

    // Give hooks time to detach safely if you implement unhooking later
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

#ifdef _DEBUG
    fclose(stdout);
    fclose(stderr);
    FreeConsole();
#endif

    if (g_module)
        FreeLibraryAndExitThread(g_module, EXIT_SUCCESS);
}

BOOL WINAPI DllMain(HMODULE module, DWORD call_reason, PVOID reserve)
{
    if (call_reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(module);
        HANDLE thread = CreateThread(nullptr, 0, install_thread, module, 0, nullptr);
        if (thread)
            CloseHandle(thread);
    }
    else if (call_reason == DLL_PROCESS_DETACH)
    {
        g_unloading.store(true);
        // If you need clean unhooking, implement g_hooking_manager->unhook_all()
    }

    return TRUE;
}
