#pragma once
#include <windows.h>
#include <cstdint>

// CS:GO 2014-10-23 build (engine.dll 13:22:52 Oct 15 2014, client.dll Oct 23 2014 22:08:35)
// VClient016, 12MB client.dll
// Reversed offsets:
// ClientState sig: A1 ? ? ? ? 8B 80 ? ? ? ? C3
// m_nChokedCommands = 0x4D30 (not 0x4CB0 old)
// m_NetChannel = 0x9C

class CClientState
{
public:
    char pad_0x0000[0x9C];
    void* m_NetChannel;                 // 0x9C
    char pad_0x00A0[0x68];
    int m_nSignonState;                 // 0x108
    char pad_0x010C[0x4C20];
    int m_nLastOutgoingCommand;         // 0x4D2C
    int m_nChokedCommands;              // 0x4D30
    int m_nLastCommandAck;              // 0x4D34
    int m_nCommandAck;                  // 0x4D38
};

class INetChannel
{
public:
    char pad_0x0000[0x18];
    int m_nOutSequenceNr;
    int m_nInSequenceNr;
    int m_nOutSequenceNrAck;
    int m_nOutReliableState;
    int m_nInReliableState;
    int m_nChokedPackets;               // 0x2C
};

// Fake lag manager for 2014 - based on 2018 code but simplified (no tickbase, no LBY)
// In 2014 max choke = 14 (sv_maxusrcmdprocessticks 16, but 14 safe)

class c_packet_manager
{
public:
    c_packet_manager();
    ~c_packet_manager();

    void initialize();
    void update(bool* send_packet_ptr);

    // bSendPacket control (from CL_Move)
    void set_send_packet(bool state);
    bool get_send_packet();
    
    // Choked commands
    int get_choked_commands();
    int get_choked_packets_netchannel();
    bool can_choke(int max_choke = 14);
    void force_send();

    CClientState* get_client_state();
    INetChannel* get_net_channel();

    // Fake lag logic (adapted from 2018 code for 2014)
    bool ShouldFakeLag(class CUserCmd* pCmd);
    int DetermineFakeLagAmount(class CUserCmd* pCmd);
    void HandleFakeLag(bool* bSendPacket, class CUserCmd* pCmd);

    // State
    bool* m_bSendPacket = nullptr;
    CClientState** m_client_state_ptr = nullptr;
    bool m_initialized = false;
    
    int m_choked_commands = 0;
    bool m_send_packet = true;
    
    // Fake lag vars (from 2018 code)
    int m_iOverrideLagAmount = -1;
    int m_iLastChokedCommands = 0;
    int m_iLagLimit = 14; // 2014 limit
    int m_iAwaitingChoke = 0;
    bool m_bReachedMaxLag = false;
};

extern c_packet_manager* g_packet_manager;
