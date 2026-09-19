#pragma once
#include "../math/QAngle.hpp"
#include "../interfaces/classes/CInput.hpp"
#include <deque>

class CUserCmd;

// Reversed logic for fake angles in 2014 build
// Classic 2014 desync method:
// - When bSendPacket = false, server doesn't receive angle -> fake for enemies
// - When bSendPacket = true, server receives angle -> real
// - Max desync delta in 2014 was ~58 degrees (from CCSGOPlayerAnimState)
// - We can achieve 180 fake via packet choking + LBY breaking

enum class EAntiAimMode
{
    NONE = 0,
    BACKWARDS,
    SIDEWAYS,
    STATIC_180,
    JITTER,
    DESYNC_58,
    LBY_BREAKER
};

enum class EAntiAimPitch
{
    NONE = 0,
    DOWN,
    UP,
    ZERO_AA,
    JITTER_PITCH
};

class c_antiaim
{
public:
    c_antiaim();
    ~c_antiaim();

    // Main entry from CreateMove
    void instance(CUserCmd* cmd, bool& send_packet);

    // Angle generators
    QAngle get_real_angle(QAngle original);
    QAngle get_fake_angle(QAngle original);
    QAngle get_real_angle();
    QAngle get_fake_angle();
    float get_max_desync_delta();

    // Helpers
    void run_yaw(CUserCmd* cmd, bool& send_packet);
    void run_pitch(CUserCmd* cmd);
    void run_lby_breaker(CUserCmd* cmd, bool& send_packet);
    
    // State tracking for packet manager
    QAngle m_real_angle = QAngle(0, 0, 0);
    QAngle m_fake_angle = QAngle(0, 0, 0);
    QAngle m_last_real_angle = QAngle(0, 0, 0);
    QAngle m_last_fake_angle = QAngle(0, 0, 0);
    
    // For LBY breaker
    float m_next_lby_update = 0.0f;
    bool m_should_break_lby = false;
    int m_choked_ticks = 0;
    
    // For jitter
    bool m_jitter_side = false;
    
    // Config
    EAntiAimMode m_yaw_mode = EAntiAimMode::DESYNC_58;
    EAntiAimPitch m_pitch_mode = EAntiAimPitch::DOWN;
    
    // Packet manager integration
    bool m_is_fake_angle_active = false;
    int m_fake_lag_ticks = 1;
};

extern c_antiaim* g_antiaim;
