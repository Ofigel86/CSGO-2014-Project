#include "../hooks.hpp"
#include "../../hacks/movement.hpp"
#include "../../hacks/ragebot.hpp"
#include "../../hacks/legitbot.hpp"
#include "../../hacks/nospread.hpp"
#include "../../hacks/antiaim.hpp"
#include "../../utilities/managers/packet_manager.hpp"

void __fastcall hk_create_move(void* ecx, void* edx, int sequence_number, float input_sample_time, bool active)
{
    static auto create_move_original = g_hooking_manager->client_dll_table ? 
        g_hooking_manager->client_dll_table->get_func_address<c_hooking::create_move_fn>(21) : nullptr;

    if (!create_move_original)
        return;

    DWORD frame_ptr = 0;

#if defined(_MSC_VER) && !defined(__clang__)
    __asm
    {
        push active
        push input_sample_time
        push sequence_number
        call create_move_original
        mov frame_ptr, ebp
    }
#else
    // For non-MSVC or Clang, call directly (less accurate but safe)
    if (create_move_original)
        create_move_original(ecx, sequence_number, input_sample_time, active);
#endif

    if (!g_interfaces || !g_context || !g_interfaces->get_input() || !g_interfaces->get_client_entity_list() || !g_interfaces->get_engine_client())
        return;

    PBYTE send_packet_ptr = nullptr;
    bool* bSendPacket = nullptr;
    __try {
        if (frame_ptr)
        {
            send_packet_ptr = reinterpret_cast<PBYTE>(*reinterpret_cast<PDWORD>(frame_ptr) - 0x1);
            bSendPacket = reinterpret_cast<bool*>(send_packet_ptr);
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) { send_packet_ptr = nullptr; bSendPacket = nullptr; }

    // Update packet manager with bSendPacket pointer (reversed from CL_Move)
    if (g_packet_manager && bSendPacket)
    {
        __try {
            g_packet_manager->update(bSendPacket);
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    CUserCmd* command = nullptr;
    CVerifiedUserCmd* verified_command = nullptr;

    __try {
        command = g_interfaces->get_input()->GetUserCmd(sequence_number);
        verified_command = g_interfaces->get_input()->GetVerifiedUserCmd(sequence_number);
    } __except(EXCEPTION_EXECUTE_HANDLER) { return; }

    if (!command || !command->m_command_number)
        return;
    if (!verified_command)
        return;

    __try {
        g_context->local_player = reinterpret_cast<c_cs_player*>(g_interfaces->get_client_entity_list()->GetClientEntity(g_interfaces->get_engine_client()->GetLocalPlayer()));
    } __except(EXCEPTION_EXECUTE_HANDLER) { g_context->local_player = nullptr; return; }

    if (!g_context->local_player)
        return;

    __try {
        auto handle = g_context->local_player->m_active_weapon();
        g_context->local_weapon = reinterpret_cast<c_base_combat_weapon*>(g_interfaces->get_client_entity_list()->GetClientEntityFromHandle(handle));
    } __except(EXCEPTION_EXECUTE_HANDLER) { g_context->local_weapon = nullptr; }

    g_context->old_angle = command->m_viewangles;

    if (g_movement)
    {
        g_movement->bunny_hop(command);
        g_movement->auto_strafe(command);
    }

    // Packet manager + AntiAim (Fake Angle) - reversed from client.dll
    // In 2014 build, fake angles work by choking packets with bSendPacket = false
    bool bSendPacketState = true;
    if (bSendPacket)
        bSendPacketState = *bSendPacket;

    if (g_antiaim)
    {
        __try {
            g_antiaim->instance(command, bSendPacketState);
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    // Apply bSendPacket state back to game (packet manager)
    if (bSendPacket && g_packet_manager)
    {
        __try {
            *bSendPacket = bSendPacketState;
            g_packet_manager->set_send_packet(bSendPacketState);
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    // Fake lag management (part of packet manager)
    if (g_packet_manager && g_variables && g_variables->antiaim_fakelag_enabled)
    {
        __try {
            int choked = g_packet_manager->get_choked_commands();
            if (choked < g_variables->antiaim_fakelag_ticks)
            {
                if (bSendPacket)
                    *bSendPacket = false;
            }
            else
            {
                if (bSendPacket)
                    *bSendPacket = true;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    if (g_ragebot)
        g_ragebot->instance(command);
    if (g_legitbot)
        g_legitbot->instance(command);
    if (g_nospread)
        g_nospread->instance(command);

    if (g_movement)
        g_movement->fix_movement(command);

    __try {
        verified_command->m_cmd = *command;
        verified_command->m_crc = command->GetChecksum();
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void __fastcall hk_frame_stage_notify(void* ecx, void* edx, ClientFrameStage_t stage)
{
    static auto frame_stage_notify_original = g_hooking_manager->client_dll_table ?
        g_hooking_manager->client_dll_table->get_func_address<c_hooking::frame_stage_notify_fn>(36) : nullptr;

    if (!frame_stage_notify_original)
        return;

    if (g_keybinds)
    {
        __try { g_keybinds->handle_toggled_keybinds(); } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    QAngle* punch_angle = nullptr;
    QAngle stored_punch_angle{};

    if (stage == FRAME_RENDER_START)
    {
        if (g_context && g_context->local_player && g_variables)
        {
            __try {
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
                {
                    if (g_context->local_player->is_alive() && g_context->local_player->m_flash_duration() > 0.0f)
                        g_context->local_player->m_flash_duration() = 0.0f;
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) { punch_angle = nullptr; }
        }
    }

    __try {
        frame_stage_notify_original(ecx, stage);
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    if (punch_angle)
    {
        __try { *punch_angle = stored_punch_angle; } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
}

void c_hooking::initialize_client_dll()
{
    if (!client_dll_table)
        return;
    client_dll_table->hook_function(reinterpret_cast<uintptr_t>(hk_create_move), 21);
    client_dll_table->hook_function(reinterpret_cast<uintptr_t>(hk_frame_stage_notify), 36);
}
