#include "packet_manager.hpp"
#include "../../utilities/scanners/signature_scanner.hpp"
#include "../../interfaces/interfaces.hpp"
#include <windows.h>

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

    // Find ClientState via signature
    // Pattern from UC: A1 ? ? ? ? 8B 80 ? ? ? ? C3
    // This is in engine.dll, points to ClientState pointer
    __try {
        auto client_state_sig = g_scanners ? g_scanners->find_signature("engine.dll", "A1 ? ? ? ? 8B 80 ? ? ? ? C3") : 0;
        if (client_state_sig)
        {
            // A1 = mov eax, [ClientState]
            // +1 is the pointer to ClientState ptr
            m_client_state_ptr = *reinterpret_cast<CClientState***>(client_state_sig + 1);
        }
        else
        {
            // Fallback: try alternative pattern used in 2014-2015
            // 8B 0D ? ? ? ? 8B D6 8B C1
            auto alt_sig = g_scanners ? g_scanners->find_signature("engine.dll", "A1 ? ? ? ? 33 D2 6A 00 6A 00 33 C9 89 B0") : 0;
            if (alt_sig)
                m_client_state_ptr = *reinterpret_cast<CClientState***>(alt_sig + 1);
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        m_client_state_ptr = nullptr;
    }

    m_initialized = true;
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
            m_last_choked = m_choked_commands;
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
    // In CS:GO 2014, max choked is limited by sv_maxusrcmdprocessticks (16) and cl_cmdrate
    // Typically we allow up to 14 for safety, 16 max
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
    if (!client_state)
        return nullptr;

    __try {
        return reinterpret_cast<INetChannel*>(client_state->m_NetChannel);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}
