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
//
// IMPORTANT 2014 LIMIT: max fake = ~90 degrees TOTAL, not 90 from real
// If you set fake >90, real starts to move away (engine clamp)
// So max desync in 2014 = 90 deg. 180 will move real! Must clamp to 90.
// Later (2017+) max desync became 58 deg, but in 2014 it's 90.
#define MAX_DESYNC_2014 90.0f
#define MAX_JITTER_HALF_2014 45.0f // for jitter both sides: +45/-45 = 90 diff

enum class EAntiAimYaw
{
    NONE = 0,
    BACKWARDS,      // 90 in 2014 (180 would move real)
    SIDEWAYS,       // 90 / -90 (max)
    STATIC_180,     // Real 90, Fake 0 in 2014 (180 would move real, so clamp to 90)
    JITTER,         // Switch +range/-range, but total diff clamped to 90
    DESYNC,         // Simple desync (real 0, fake +range clamped to 90)
    SPIN            // Spinbot with 90 max desync
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
    bool m_jitter_side_pitch = false;
    float m_spin_yaw = 0.0f;
    int m_jitter_tick = 0;
    
    EAntiAimYaw m_yaw_mode = EAntiAimYaw::DESYNC;
    EAntiAimPitch m_pitch_mode = EAntiAimPitch::DOWN;
    
    // Configurable jitter radius - default jitters at max speed
    float m_jitter_range = 45.0f; // yaw jitter radius
    float m_jitter_range_pitch = 10.0f; // pitch jitter radius
    bool m_jitter_random = false;
    int m_jitter_speed = 1; // 1 = every tick (max speed)
    
    bool m_is_active = false;
};

extern c_antiaim* g_antiaim;
