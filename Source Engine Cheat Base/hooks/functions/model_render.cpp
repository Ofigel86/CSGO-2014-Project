#include "../hooks.hpp"

IMaterial* create_material(bool lit, const std::string& material_data)
{
	static auto created = 0;
	std::string type = lit ? "VertexLitGeneric" : "UnlitGeneric";

	auto matname = "csgosdkv2_" + std::to_string(created);
	++created;

	auto keyValues = new KeyValues(matname.c_str());
	static auto key_values_address = g_scanners->find_signature("client.dll", "55 8B EC 51 33 C0 C7 45");

	using KeyValuesFn = void(__thiscall*)(void*, const char*);
	reinterpret_cast <KeyValuesFn> (key_values_address)(keyValues, type.c_str());

	static auto load_from_buffer_address = g_scanners->find_signature("client.dll", "55 8B EC 83 E4 F8 83 EC 34 53 8B 5D 0C 89");
	using LoadFromBufferFn = void(__thiscall*)(void*, const char*, const char*, void*, const char*, void*);

	reinterpret_cast <LoadFromBufferFn> (load_from_buffer_address)(keyValues, matname.c_str(), material_data.c_str(), nullptr, nullptr, nullptr);

	auto material = g_interfaces->get_material_system()->CreateMaterial(matname.c_str(), keyValues);
	material->IncrementReferenceCount();

	return material;
}

void __stdcall hk_draw_model_execute(void* rendercontext, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
{
	static auto original_fn = g_hooking_manager->model_render_table->get_func_address<c_hooking::draw_model_execute_fn>(21);

	auto model_entity = reinterpret_cast<c_cs_player*>(g_interfaces->get_client_entity_list()->GetClientEntity(pInfo.entity_index));
	auto name = g_interfaces->get_model_info()->GetModelName(pInfo.pModel);

	auto is_player = strstr(name, "models/player") && model_entity->is_alive() && g_context->local_player->is_alive();

	if (!is_player)
		return original_fn(g_interfaces->get_model_render(), rendercontext, state, pInfo, pCustomBoneToWorld);

	if (g_interfaces->get_model_render()->IsForcedMaterialOverride())
		return original_fn(g_interfaces->get_model_render(), rendercontext, state, pInfo, pCustomBoneToWorld);

	static IMaterial* materials[] =
	{
		create_material(true, (R"#("VertexLitGeneric"
			{
				"$basetexture"				"vgui/white"
				"$ignorez"					"0"
				"$envmap"					" "
				"$nofog"					"1"
				"$model"					"1"
				"$nocull"					"0"
				"$selfillum"				"1"
				"$halflambert"				"1"
				"$znearer"					"0"
				"$flat"						"0"
				"$wireframe"				"0"
			}
		)#")),
		create_material(true, (R"#("VertexLitGeneric" 
			{
				"$basetexture"				"vgui/white" 
				"$ignorez"					"0" 
				"$envmap"					"env_cubemap" 
				"$normalmapalphaenvmapmask" "1" 
				"$envmapcontrast"			"1" 
				"$nofog"					"1" 
				"$model"					"1" 
				"$nocull" 					"0" 
				"$selfillum" 				"1" 
				"$halflambert"				"1" 
				"$znearer" 					"0" 
				"$flat" 					"1"
		        "$wireframe"				"0"
			}
		)#")),
		create_material(false, (R"#("UnlitGeneric"
			{
				"$basetexture"				"vgui/white"
				"$ignorez"					"0"
				"$envmap"					" "
				"$nofog"					"1"
				"$model"					"1"
				"$nocull"					"0"
				"$selfillum"				"1"
				"$halflambert"				"1"
				"$znearer"					"0"
				"$flat"						"1"
				"$wireframe"				"0"
			}
		)#"))
	};

	auto called_original = false;

	if (g_variables->chams_enabled)
	{
		if (is_player)
		{
			if (model_entity->m_team() != g_context->local_player->m_team())
			{
				auto material = materials[g_variables->chams_material];

				if (material && g_variables->chams_enemy_enabled)
				{
					g_interfaces->get_render_view()->SetBlend(g_variables->chams_enemy_color_invisible[3]);
					g_interfaces->get_render_view()->SetColorModulation(g_variables->chams_enemy_color_invisible);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

					g_interfaces->get_model_render()->ForcedMaterialOverride(material);
					original_fn(g_interfaces->get_model_render(), rendercontext, state, pInfo, pCustomBoneToWorld);
					g_interfaces->get_model_render()->ForcedMaterialOverride(NULL);

					g_interfaces->get_render_view()->SetBlend(g_variables->chams_enemy_color_visible[3]);
					g_interfaces->get_render_view()->SetColorModulation(g_variables->chams_enemy_color_visible);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

					g_interfaces->get_model_render()->ForcedMaterialOverride(material);
					original_fn(g_interfaces->get_model_render(), rendercontext, state, pInfo, pCustomBoneToWorld);
					g_interfaces->get_model_render()->ForcedMaterialOverride(NULL);

					called_original = true;
				}
			}
		}
	}

	if (!called_original)
		original_fn(g_interfaces->get_model_render(), rendercontext, state, pInfo, pCustomBoneToWorld);

	called_original = false;
}

void c_hooking::initialize_model_render()
{
	model_render_table->hook_function(reinterpret_cast<uintptr_t>(hk_draw_model_execute), 21);
}