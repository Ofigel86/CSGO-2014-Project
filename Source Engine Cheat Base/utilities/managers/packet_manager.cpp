#include "packet_manager.hpp"
#include "../../utilities/scanners/signature_scanner.hpp"
#include "../../interfaces/interfaces.hpp"
#include "../../game/classes/entity.hpp"
#include "../../config/config.hpp"
#include <windows.h>
#include <algorithm>
#include <cmath>

c_packet_manager* g_packet_manager = new c_packet_manager();

c_packet_manager::c_packet_manager()
{
}

c_packet_manager::~c_packet_manager()
{
}

void c_packet_manager::initialize()
{
    if (m_initialized)
        return;

    __try {
        auto client_state_sig = g_scanners ? g_scanners->find_signature("engine.dll", "A1 ? ? ? ? 8B 80 ? ? ? ? C3") : 0;
        if (client_state_sig)
        {
            m_client_state_ptr = *reinterpret_cast<CClientState***>(client_state_sig + 1);
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        m_client_state_ptr = nullptr;
    }

    m_initialized = true;
    m_iLagLimit = 14; // 2014 max
}

void c_packet_manager::update(bool* send_packet_ptr)
{
    if (!m_initialized)
        initialize();

    if (send_packet_ptr)
        m_bSendPacket = send_packet_ptr;

    auto client_state = get_client_state();
    if (client_state)
    {
        __try {
            m_choked_commands = client_state->m_nChokedCommands;
            m_iLastChokedCommands = m_choked_commands;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            m_choked_commands = 0;
        }
    }

    if (m_bSendPacket)
    {
        __try {
            m_send_packet = *m_bSendPacket;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            m_send_packet = true;
        }
    }
}

void c_packet_manager::set_send_packet(bool state)
{
    if (m_bSendPacket)
    {
        __try {
            *m_bSendPacket = state;
            m_send_packet = state;
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
}

bool c_packet_manager::get_send_packet()
{
    if (m_bSendPacket)
    {
        __try {
            return *m_bSendPacket;
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
    return true;
}

int c_packet_manager::get_choked_commands()
{
    auto client_state = get_client_state();
    if (client_state)
    {
        __try {
            return client_state->m_nChokedCommands;
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
    return m_choked_commands;
}

int c_packet_manager::get_choked_packets_netchannel()
{
    auto netchan = get_net_channel();
    if (netchan)
    {
        __try {
            return netchan->m_nChokedPackets;
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
    return 0;
}

bool c_packet_manager::can_choke(int max_choke)
{
    int choked = get_choked_commands();
    return choked < max_choke;
}

void c_packet_manager::force_send()
{
    set_send_packet(true);
}

CClientState* c_packet_manager::get_client_state()
{
    if (!m_client_state_ptr)
        return nullptr;
    __try {
        return *m_client_state_ptr;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

INetChannel* c_packet_manager::get_net_channel()
{
    auto client_state = get_client_state();
    if (!client_state) return nullptr;
    __try {
        return reinterpret_cast<INetChannel*>(client_state->m_NetChannel);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

// Fake lag for 2014 - simplified from 2018 code
// In 2014 there is NO LBY, no tickbase shift, simpler logic
// Based on provided 2018 code but adapted

bool c_packet_manager::ShouldFakeLag(CUserCmd* pCmd)
{
    if (!g_context || !g_context->local_player || !pCmd)
        return false;

    auto pLocal = g_context->local_player;
    
    __try {
        if (!pLocal->is_alive())
            return false;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return false; }

    bool bReturnValue = false;

    if (!g_variables || !g_variables->antiaim_fakelag_enabled)
        return false;

    __try {
        int flags = pLocal->m_flags();
        bool onGround = (flags & (1 << 0)); // FL_ONGROUND
        Vector vel = pLocal->m_velocity();
        float velLen2D = vel.Length2D();
        float velLen = vel.Length();

        // Moving check
        if (velLen2D > 0.1f && onGround)
            bReturnValue = true;

        // Accelerate check (from 2018 code)
        static float flPreviousVelocity = 0.f;
        if (flPreviousVelocity != velLen)
        {
            if (flPreviousVelocity < velLen)
                bReturnValue = true;
            flPreviousVelocity = velLen;
        }

        // Air check
        if (!(flags & (1 << 0)))
            bReturnValue = true;

        // Ladder check
        int moveType = pLocal->m_move_type();
        if (moveType == 9) // MOVETYPE_LADDER = 9
            bReturnValue = true;

        // Unduck check (from 2018)
        float duckAmount = pLocal->m_duck_amount();
        if (duckAmount > 0.0f && duckAmount < 1.0f)
        {
            static float flPrevDuck = duckAmount;
            if (flPrevDuck != duckAmount && duckAmount < flPrevDuck)
                bReturnValue = true;
            flPrevDuck = duckAmount;
        }

        // If standing still, still fakelag for desync
        if (velLen2D <= 0.1f && onGround)
            bReturnValue = true;

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        bReturnValue = true;
    }

    return bReturnValue;
}

int c_packet_manager::DetermineFakeLagAmount(CUserCmd* pCmd)
{
    if (!g_variables)
        return 1;

    // For 2014, simple: use config value
    int iLagAmount = g_variables->antiaim_fakelag_ticks;
    
    // Clamp to limit
    iLagAmount = std::clamp(iLagAmount, 1, m_iLagLimit);

    // Dynamic type based on velocity (if we had it)
    // For 2014, just use static or simple dynamic
    
    // Apply variance if needed (from 2018 code)
    // For 2014, skip variance for simplicity

    return m_iOverrideLagAmount != -1 ? m_iOverrideLagAmount : iLagAmount;
}

void c_packet_manager::HandleFakeLag(bool* bSendPacket, CUserCmd* pCmd)
{
    m_bReachedMaxLag = false;

    if (!bSendPacket || !pCmd)
        return;

    if (!g_variables || !g_variables->antiaim_fakelag_enabled)
        return;

    auto pLocal = g_context ? g_context->local_player : nullptr;
    if (!pLocal)
        return;

    __try {
        if (!pLocal->is_alive())
            return;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return; }

    // Don't fakelag when using or attacking with grenade
    if (pCmd->m_buttons & IN_USE)
        return;

    // Grenade check
    if (g_context->local_weapon)
    {
        __try {
            if (g_context->local_weapon->is_grenade())
            {
                // If pin pulled or throwing, send packet
                if (g_context->local_weapon->m_pin_pulled())
                {
                    if (pCmd->m_buttons & (IN_ATTACK | IN_ATTACK2))
                    {
                        // Throw time check
                        float throwTime = g_context->local_weapon->m_throw_time();
                        if (throwTime > 0.0f)
                        {
                            *bSendPacket = true;
                            return;
                        }
                    }
                }
            }
            else
            {
                // If attacking with weapon and can shoot, optionally choke shot (fake lag on shot)
                if ((pCmd->m_buttons & IN_ATTACK) && g_variables->antiaim_fakelag_enabled)
                {
                    // For 2014, don't choke shot by default, but allow
                    // *bSendPacket = false;
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    // Determine if should fakelag
    if (ShouldFakeLag(pCmd))
    {
        m_iAwaitingChoke = std::clamp<int>(DetermineFakeLagAmount(pCmd), 0, m_iLagLimit);
        *bSendPacket = false;
    }

    // Check if reached awaiting choke
    if (get_choked_commands() >= m_iAwaitingChoke)
    {
        *bSendPacket = true;
    }

    // Force send if over limit (14 for 2014, 16 max)
    m_iLagLimit = 14;

    if (get_choked_commands() >= m_iLagLimit)
    {
        *bSendPacket = true;
        m_bReachedMaxLag = true;
    }

    m_iOverrideLagAmount = -1;
}
