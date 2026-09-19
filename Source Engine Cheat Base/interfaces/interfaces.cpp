#include "interfaces.hpp"

#include <windows.h>

c_interfaces* g_interfaces = new c_interfaces();

template<typename T>
static T* get_interface(const char* mod_name, const char* interface_name, bool exact = false) {
    if (!mod_name || !interface_name)
        return nullptr;

    T* iface = nullptr;
    InterfaceReg* register_list = nullptr;
    int part_match_len = static_cast<int>(strlen(interface_name));

    HMODULE hMod = GetModuleHandleA(mod_name);
    if (!hMod)
        return nullptr;

    DWORD interface_fn = reinterpret_cast<DWORD>(GetProcAddress(hMod, "CreateInterface"));
    if (!interface_fn) {
        return nullptr;
    }

    __try
    {
        unsigned int jump_start = static_cast<unsigned int>(interface_fn) + 4;
        unsigned int jump_target = jump_start + *reinterpret_cast<unsigned int*>(jump_start + 1) + 5;
        register_list = **reinterpret_cast<InterfaceReg***>(jump_target + 6);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return nullptr;
    }

    for (InterfaceReg* cur = register_list; cur; cur = cur->m_pNext) {
        if (!cur->m_pName || !cur->m_CreateFn)
            continue;

        if (exact == true) {
            if (strcmp(cur->m_pName, interface_name) == 0)
            {
                __try { iface = reinterpret_cast<T*>(cur->m_CreateFn()); } __except(EXCEPTION_EXECUTE_HANDLER) {}
            }
        }
        else {
            if (!strncmp(cur->m_pName, interface_name, part_match_len) && std::atoi(cur->m_pName + part_match_len) > 0)
            {
                __try { iface = reinterpret_cast<T*>(cur->m_CreateFn()); } __except(EXCEPTION_EXECUTE_HANDLER) {}
            }
        }
    }
    return iface;
}

void c_interfaces::initialize()
{
    engine_client = get_interface<IVEngineClient>("engine.dll", "VEngineClient");
    client_dll = get_interface<IBaseClientDLL>("client.dll", "VClient");
    client_entity_list = get_interface<IClientEntityList>("client.dll", "VClientEntityList");
    
    if (client_dll)
    {
        __try {
            client_mode = **(IClientMode***)((*(DWORD**)client_dll)[10] + 0x5);
        } __except(EXCEPTION_EXECUTE_HANDLER) { client_mode = nullptr; }

        __try {
            global_vars = **(CGlobalVarsBase***)((*(DWORD**)(client_dll))[0] + 0x53);
        } __except(EXCEPTION_EXECUTE_HANDLER) { global_vars = nullptr; }

        __try {
            input = *reinterpret_cast<CInput**>((*reinterpret_cast<uintptr_t**>(client_dll))[15] + 0x1);
        } __except(EXCEPTION_EXECUTE_HANDLER) { input = nullptr; }
    }

    vgui_surface = get_interface<ISurface>("vguimatsurface.dll", "VGUI_Surface");
    vgui_panel = get_interface<IPanel>("vgui2.dll", "VGUI_Panel");
    render_view = get_interface<IVRenderView>("engine.dll", "VEngineRenderView");
    cvars = get_interface<ICvar>("vstdlib.dll", "VEngineCvar");
    model_render = get_interface<IVModelRender>("engine.dll", "VEngineModel");
    material_system = get_interface<IMaterialSystem>("materialsystem.dll", "VMaterialSystem");
    model_info = get_interface<IModelInfo>("engine.dll", "VModelInfoClient0");
    engine_trace = get_interface<IEngineTrace>("engine.dll", "EngineTraceClient");
    physics_surface = get_interface<IPhysicsSurfaceProps>("vphysics.dll", "VPhysicsSurfaceProps");

    auto shaderapidx9_sig = g_scanners->find_signature("shaderapidx9.dll", "A1 ? ? ? ? 50 8B 08 FF 51 0C");
    if (shaderapidx9_sig)
    {
        __try {
            directx = **(IDirect3DDevice9***)(shaderapidx9_sig + 0x1);
        } __except(EXCEPTION_EXECUTE_HANDLER) { directx = nullptr; }
    }
}

void c_interfaces::initialize_netvars()
{
    netvars::get().tables.clear();
    if (!client_dll)
        return;

    auto client = client_dll->GetAllClasses();
    if (!client)
        return;

    while (client)
    {
        auto recvTable = client->m_pRecvTable;
        if (recvTable && client->m_pNetworkName)
            netvars::get().tables.emplace(std::string(client->m_pNetworkName), recvTable);

        client = client->m_pNext;
    }
}

IVEngineClient* c_interfaces::get_engine_client() { return engine_client; }
IBaseClientDLL* c_interfaces::get_client_dll() { return client_dll; }
IClientEntityList* c_interfaces::get_client_entity_list() { return client_entity_list; }
IClientMode* c_interfaces::get_client_mode() { return client_mode; }
ISurface* c_interfaces::get_vgui_surface() { return vgui_surface; }
IPanel* c_interfaces::get_vgui_panel() { return vgui_panel; }
IVRenderView* c_interfaces::get_render_view() { return render_view; }
ICvar* c_interfaces::get_cvars() { return cvars; }
IVModelRender* c_interfaces::get_model_render() { return model_render; }
IModelInfo* c_interfaces::get_model_info() { return model_info; }
IMaterialSystem* c_interfaces::get_material_system() { return material_system; }
IEngineTrace* c_interfaces::get_engine_trace() { return engine_trace; }
CGlobalVarsBase* c_interfaces::get_global_vars() { return global_vars; }
IPhysicsSurfaceProps* c_interfaces::get_physics_surface() { return physics_surface; }
CInput* c_interfaces::get_input() { return input; }
IDirect3DDevice9* c_interfaces::get_directx_device() { return directx; }
