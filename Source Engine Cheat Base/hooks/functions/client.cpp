#include "../hooks.hpp"
#include "../../hacks/movement.hpp"
#include "../../hacks/ragebot.hpp"
#include "../../hacks/legitbot.hpp"
#include "../../hacks/nospread.hpp"

void __fastcall hk_create_move(void* ecx, void* edx, int sequence_number, float input_sample_time, bool active)
{
	static auto create_move_original = g_hooking_manager->client_dll_table->get_func_address<c_hooking::create_move_fn>(21);

	DWORD frame_ptr = 0x00000000;

	__asm
	{
		push active
		push input_sample_time
		push sequence_number
		call create_move_original
		mov frame_ptr, ebp
	}

	PBYTE send_packet_ptr = (PBYTE)(*(PDWORD)(frame_ptr) - 0x1);

	CUserCmd* command = g_interfaces->get_input()->GetUserCmd(sequence_number);
	CVerifiedUserCmd* verified_command = g_interfaces->get_input()->GetVerifiedUserCmd(sequence_number);

	if (!command || !command->m_command_number)
		return;

	if (!verified_command)
		return;

	g_context->local_player = reinterpret_cast<c_cs_player*>(g_interfaces->get_client_entity_list()->GetClientEntity(g_interfaces->get_engine_client()->GetLocalPlayer()));

	if (!g_context->local_player)
		return;

	g_context->local_weapon = reinterpret_cast<c_base_combat_weapon*>(g_interfaces->get_client_entity_list()->GetClientEntityFromHandle(g_context->local_player->m_active_weapon()));

	g_context->old_angle = command->m_viewangles;

	g_movement->bunny_hop(command);
	g_movement->auto_strafe(command);

	g_ragebot->instance(command);
	g_legitbot->instance(command);

	g_nospread->instance(command);

	g_movement->fix_movement(command);

	verified_command->m_cmd = *command;
	verified_command->m_crc = command->GetChecksum();
}

void __fastcall hk_frame_stage_notify(void* ecx, void* edx, ClientFrameStage_t stage)
{
	static auto frame_stage_notify_original = g_hooking_manager->client_dll_table->get_func_address<c_hooking::frame_stage_notify_fn>(36);

	g_keybinds->handle_toggled_keybinds();

	QAngle* punch_angle = nullptr;
	QAngle stored_punch_angle;

	if (stage == FRAME_RENDER_START)
	{
		if (g_variables->removals_visual_recoil && g_context->local_player->is_alive())
		{
			punch_angle = &g_context->local_player->m_aim_punch_angle();

			if (punch_angle)
			{
				stored_punch_angle = *punch_angle;
				punch_angle->Init();
			}
		}

		if (g_variables->removals_flash)
			if (g_context->local_player->is_alive() && g_context->local_player->m_flash_duration() > 0.0)
				g_context->local_player->m_flash_duration() = 0.0;
	}

	frame_stage_notify_original(ecx, stage);

	if (punch_angle)
		*punch_angle = stored_punch_angle;
}

void c_hooking::initialize_client_dll()
{
	client_dll_table->hook_function(reinterpret_cast<uintptr_t>(hk_create_move), 21);
	client_dll_table->hook_function(reinterpret_cast<uintptr_t>(hk_frame_stage_notify), 36);
}