#pragma once
#include <windows.h>
#include <cstdint>

// Reversed from engine.dll 2014-10-23 build
// ClientState pattern: A1 ? ? ? ? 8B 80 ? ? ? ? C3 -> ptr to ClientState
// Offsets verified via disasm and UC threads:
// - m_nChokedCommands at 0x4D30 (was 0x4CB0 in older builds, 0x4D30 in 2014)
// - m_nLastOutgoingCommand at 0x4D2C
// - m_NetChannel at 0x9C

class CClientState
{
public:
    char pad_0x0000[0x9C];              // 0x00
    void* m_NetChannel;                 // 0x9C
    char pad_0x00A0[0x68];              // 0xA0
    int m_nSignonState;                 // 0x108
    char pad_0x010C[0x4C20];            // 0x10C
    int m_nLastOutgoingCommand;         // 0x4D2C
    int m_nChokedCommands;              // 0x4D30
    int m_nLastCommandAck;              // 0x4D34
    int m_nCommandAck;                  // 0x4D38
    // ... rest not needed
};

class INetChannel
{
public:
    char pad_0x0000[0x18];
    int m_nOutSequenceNr;               // 0x18
    int m_nInSequenceNr;                // 0x1C
    int m_nOutSequenceNrAck;            // 0x20
    int m_nOutReliableState;            // 0x24
    int m_nInReliableState;             // 0x28
    int m_nChokedPackets;               // 0x2C - alternative choked counter from netchannel
};

class c_packet_manager
{
public:
    c_packet_manager();
    ~c_packet_manager();

    void initialize();
    
    // Called from CreateMove hook with bSendPacket pointer
    void update(bool* send_packet_ptr);
    
    // Packet control
    void set_send_packet(bool state);
    bool get_send_packet();
    int get_choked_commands();
    int get_choked_packets_netchannel();
    bool can_choke(int max_choke = 14);
    void force_send();
    
    // ClientState access
    CClientState* get_client_state();
    INetChannel* get_net_channel();
    
    // For fake angle: track if we are choking
    bool is_choking() { return m_choked_commands > 0 && !m_send_packet; }
    int m_choked_commands = 0;
    bool m_send_packet = true;
    
    // For debugging / packet manager logic
    int m_last_choked = 0;
    bool m_should_choke = false;
    
    // Pattern scanned pointers
    CClientState** m_client_state_ptr = nullptr;
    bool* m_bSendPacket = nullptr;
    
    bool m_initialized = false;
};

extern c_packet_manager* g_packet_manager;
