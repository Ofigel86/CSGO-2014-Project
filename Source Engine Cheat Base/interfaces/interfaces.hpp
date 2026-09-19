#pragma once
#include "classes/IVEngineClient.hpp"
#include "classes/IBaseClientDll.hpp"
#include "classes/IClientEntity.hpp"
#include "classes/GlobalVars.hpp"
#include "classes/IHandleEntity.hpp"
#include "classes/IClientEntityList.hpp"
#include "classes/IClientMode.hpp"
#include "classes/IRenderView.hpp"
#include "classes/IEngineTrace.hpp"
#include "classes/ISurface.hpp"
#include "classes/IPanel.hpp"
#include "classes/CInput.hpp"
#include "classes/ICvar.hpp"
#include "classes/Convar.hpp"

#include "../utilities/scanners/signature_scanner.hpp"
#include "../utilities/managers/netvars_manager.hpp"

#include "../utilities/context.hpp"
#include "../utilities/math/math.hpp"

#include "../game/classes/entity.hpp"
#include "../game/managers/draw_manager.hpp"

#include "../game/enums/item_definition.hpp"
#include "../game/enums/bsp_flags.hpp"
#include "../game/enums/class_id.hpp"
#include "../game/enums/bones.hpp"

#include "../config/config.hpp"

#include <d3d9.h>

class InterfaceReg
{
private:
	using InstantiateInterfaceFn = void* (*)();
public:
	InstantiateInterfaceFn m_CreateFn;
	const char* m_pName;
	InterfaceReg* m_pNext;
};

class c_interfaces
{
	IVEngineClient* engine_client = nullptr;
	IBaseClientDLL* client_dll = nullptr;
	IClientEntityList* client_entity_list = nullptr;
	IClientMode* client_mode = nullptr;
	ISurface* vgui_surface = nullptr;
	IPanel* vgui_panel = nullptr;
	IVRenderView* render_view = nullptr;
	ICvar* cvars = nullptr;
	IVModelRender* model_render = nullptr;
	IModelInfo* model_info = nullptr;
	IMaterialSystem* material_system = nullptr;
	IEngineTrace* engine_trace = nullptr;
	CGlobalVarsBase* global_vars = nullptr;
	IPhysicsSurfaceProps* physics_surface = nullptr;
	CInput* input = nullptr;
	IDirect3DDevice9* directx = nullptr;
public:
	void initialize();
	void initialize_netvars();

	IVEngineClient* get_engine_client();
	IBaseClientDLL* get_client_dll();
	IClientEntityList* get_client_entity_list();
	IClientMode* get_client_mode();
	ISurface* get_vgui_surface();
	IPanel* get_vgui_panel();
	IVRenderView* get_render_view();
	ICvar* get_cvars();
	IVModelRender* get_model_render();
	IModelInfo* get_model_info();
	IMaterialSystem* get_material_system();
	IEngineTrace* get_engine_trace();
	CGlobalVarsBase* get_global_vars();
	IPhysicsSurfaceProps* get_physics_surface();
	CInput* get_input();
	IDirect3DDevice9* get_directx_device();
};

extern c_interfaces* g_interfaces;