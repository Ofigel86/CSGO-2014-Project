#include "../hooks.hpp"

IMaterial* create_material(bool lit, const std::string& material_data)
{
    if (!g_interfaces || !g_interfaces->get_material_system() || !g_scanners)
        return nullptr;

    static int created = 0;
    std::string type = lit ? "VertexLitGeneric" : "UnlitGeneric";

    auto matname = "csgosdkv2_" + std::to_string(created);
    ++created;

    if (created > 100) // prevent leak
        created = 0;

    KeyValues* keyValues = new KeyValues(matname.c_str());
    if (!keyValues)
        return nullptr;

    static auto key_values_address = g_scanners->find_signature("client.dll", "55 8B EC 51 33 C0 C7 45");
    static auto load_from_buffer_address = g_scanners->find_signature("client.dll", "55 8B EC 83 E4 F8 83 EC 34 53 8B 5D 0C 89");

    if (!key_values_address || !load_from_buffer_address)
    {
        delete keyValues;
        return nullptr;
    }

    __try {
        using KeyValuesFn = void(__thiscall*)(void*, const char*);
        reinterpret_cast<KeyValuesFn>(key_values_address)(keyValues, type.c_str());

        using LoadFromBufferFn = void(__thiscall*)(void*, const char*, const char*, void*, const char*, void*);
        reinterpret_cast<LoadFromBufferFn>(load_from_buffer_address)(keyValues, matname.c_str(), material_data.c_str(), nullptr, nullptr, nullptr);

        auto material = g_interfaces->get_material_system()->CreateMaterial(matname.c_str(), keyValues);
        if (material)
            material->IncrementReferenceCount();

        return material;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

void __stdcall hk_draw_model_execute(void* rendercontext, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
{
    static auto original_fn = g_hooking_manager->model_render_table ?
        g_hooking_manager->model_render_table->get_func_address<c_hooking::draw_model_execute_fn>(21) : nullptr;

    if (!original_fn)
        return;

    if (!g_interfaces || !g_context || !g_context->local_player || !g_variables)
        return original_fn(g_interfaces->get_model_render(), rendercontext, state, pInfo, pCustomBoneToWorld);

    auto model_entity = reinterpret_cast<c_cs_player*>(g_interfaces->get_client_entity_list()->GetClientEntity(pInfo.entity_index));
    auto name = g_interfaces->get_model_info() ? g_interfaces->get_model_info()->GetModelName(pInfo.pModel) : nullptr;

    if (!name || !model_entity)
        return original_fn(g_interfaces->get_model_render(), rendercontext, state, pInfo, pCustomBoneToWorld);

    bool is_player = false;
    __try {
        is_player = strstr(name, "models/player") != nullptr && model_entity->is_alive() && g_context->local_player->is_alive();
    } __except(EXCEPTION_EXECUTE_HANDLER) { is_player = false; }

    if (!is_player)
        return original_fn(g_interfaces->get_model_render(), rendercontext, state, pInfo, pCustomBoneToWorld);

    if (g_interfaces->get_model_render()->IsForcedMaterialOverride())
        return original_fn(g_interfaces->get_model_render(), rendercontext, state, pInfo, pCustomBoneToWorld);

    static IMaterial* materials[3] = { nullptr, nullptr, nullptr };
    static bool materials_created = false;

    if (!materials_created)
    {
        __try {
            materials[0] = create_material(true, (R"#("VertexLitGeneric"
                {
                    "$basetexture"              "vgui/white"
                    "$ignorez"                  "0"
                    "$envmap"                   " "
                    "$nofog"                    "1"
                    "$model"                    "1"
                    "$nocull"                   "0"
                    "$selfillum"                "1"
                    "$halflambert"              "1"
                    "$znearer"                  "0"
                    "$flat"                     "0"
                    "$wireframe"                "0"
                }
            )#"));

            materials[1] = create_material(true, (R"#("VertexLitGeneric" 
                {
                    "$basetexture"              "vgui/white" 
                    "$ignorez"                  "0" 
                    "$envmap"                   "env_cubemap" 
                    "$normalmapalphaenvmapmask" "1" 
                    "$envmapcontrast"           "1" 
                    "$nofog"                    "1" 
                    "$model"                    "1" 
                    "$nocull"                   "0" 
                    "$selfillum"                "1" 
                    "$halflambert"              "1" 
                    "$znearer"                  "0" 
                    "$flat"                     "1"
                    "$wireframe"                "0"
                }
            )#"));

            materials[2] = create_material(false, (R"#("UnlitGeneric"
                {
                    "$basetexture"              "vgui/white"
                    "$ignorez"                  "0"
                    "$envmap"                   " "
                    "$nofog"                    "1"
                    "$model"                    "1"
                    "$nocull"                   "0"
                    "$selfillum"                "1"
                    "$halflambert"              "1"
                    "$znearer"                  "0"
                    "$flat"                     "1"
                    "$wireframe"                "0"
                }
            )#"));
            materials_created = true;
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    auto called_original = false;

    if (g_variables->chams_enabled)
    {
        __try {
            if (is_player)
            {
                if (model_entity->m_team() != g_context->local_player->m_team())
                {
                    int mat_idx = g_variables->chams_material;
                    if (mat_idx < 0 || mat_idx >= 3) mat_idx = 0;
                    auto material = materials[mat_idx];

                    if (material && g_variables->chams_enemy_enabled)
                    {
                        auto render_view = g_interfaces->get_render_view();
                        auto model_render = g_interfaces->get_model_render();
                        if (render_view && model_render)
                        {
                            render_view->SetBlend(g_variables->chams_enemy_color_invisible[3]);
                            render_view->SetColorModulation(g_variables->chams_enemy_color_invisible);

                            material->IncrementReferenceCount();
                            material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

                            model_render->ForcedMaterialOverride(material);
                            original_fn(g_interfaces->get_model_render(), rendercontext, state, pInfo, pCustomBoneToWorld);
                            model_render->ForcedMaterialOverride(nullptr);

                            render_view->SetBlend(g_variables->chams_enemy_color_visible[3]);
                            render_view->SetColorModulation(g_variables->chams_enemy_color_visible);

                            material->IncrementReferenceCount();
                            material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

                            model_render->ForcedMaterialOverride(material);
                            original_fn(g_interfaces->get_model_render(), rendercontext, state, pInfo, pCustomBoneToWorld);
                            model_render->ForcedMaterialOverride(nullptr);

                            called_original = true;
                        }
                    }
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) { called_original = false; }
    }

    if (!called_original)
    {
        __try {
            original_fn(g_interfaces->get_model_render(), rendercontext, state, pInfo, pCustomBoneToWorld);
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
}

void c_hooking::initialize_model_render()
{
    if (!model_render_table)
        return;
    model_render_table->hook_function(reinterpret_cast<uintptr_t>(hk_draw_model_execute), 21);
}
