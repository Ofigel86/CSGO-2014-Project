#pragma once
#include "../math/QAngle.hpp"
#include "../interfaces/classes/CInput.hpp"

// Fix for Vector.hpp #define ZERO macro that breaks enum
#ifdef ZERO
#undef ZERO
#endif

class CUserCmd;

// 2014 CS:GO - NO LBY! LBY was added in 2015-2016 update
// In 2014 fake angle is done via simple bSendPacket choke
// Real = what server sees (when bSendPacket=true)
// Fake = what enemies see (when bSendPacket=false, choked)

enum class EAntiAimYaw
{
    NONE = 0,
    BACKWARDS,      // 180
    SIDEWAYS,       // 90 / -90
    STATIC_180,     // Real 180, Fake 0
    JITTER,         // Switch 90/-90 each tick
    DESYNC,         // Simple desync (real 0, fake +58 or +180)
    SPIN            // Spinbot
};

enum class EAntiAimPitch
{
    NONE = 0,
    DOWN,           // 89
    UP,             // -89
    ZERO_AA,        // 0 - renamed from ZERO to avoid macro conflict
    JITTER_PITCH    // 89/-89 jitter
};

class c_antiaim
{
public:
    c_antiaim();
    ~c_antiaim();

    void instance(CUserCmd* cmd, bool& send_packet);

    void run_pitch(CUserCmd* cmd);
    void run_yaw(CUserCmd* cmd, bool& send_packet);

    QAngle m_real_angle = QAngle(0, 0, 0);
    QAngle m_fake_angle = QAngle(0, 0, 0);
    
    bool m_jitter_side = false;
    float m_spin_yaw = 0.0f;
    
    EAntiAimYaw m_yaw_mode = EAntiAimYaw::DESYNC;
    EAntiAimPitch m_pitch_mode = EAntiAimPitch::DOWN;
    
    bool m_is_active = false;
};

extern c_antiaim* g_antiaim;
