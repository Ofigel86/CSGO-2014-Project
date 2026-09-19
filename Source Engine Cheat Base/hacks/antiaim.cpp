#include "antiaim.hpp"
#include "../config/config.hpp"
#include "../interfaces/interfaces.hpp"
#include "../utilities/managers/packet_manager.hpp"
#include "../game/classes/entity.hpp"
#include "../utilities/math/math.hpp"
#include <windows.h>

// Reversed from client.dll 2014-10-23
// CCSGOPlayerAnimState::GetMaxDesyncDelta is at 0x... in 2014 it's ~58
// We can calculate or use static 58 for this build

c_antiaim* g_antiaim = new c_antiaim();

c_antiaim::c_antiaim()
{
    m_yaw_mode = EAntiAimMode::DESYNC_58;
    m_pitch_mode = EAntiAimPitch::DOWN;
}

c_antiaim::~c_antiaim()
{
}

float c_antiaim::get_max_desync_delta()
{
    // In 2014 build, max desync is calculated from CCSGOPlayerAnimState
    // Reversed: if speed < 0.1, max = 58, else based on duck amount etc
    // For 2014-10-23, typical value is 58
    // TODO: reverse animstate properly: animstate + 0x148 + etc
    // For now return static 58, but try to get from entity if possible
    
    if (g_context && g_context->local_player)
    {
        __try {
            // Try to get velocity via origin delta or netvar if available
            // Fallback: use 58 static for now, will be updated when velocity netvar added
            // Vector vel = g_context->local_player->m_origin() - ... 
            // For now assume standing
            return 58.0f;
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
    
    return 58.0f;
}

void c_antiaim::instance(CUserCmd* cmd, bool& send_packet)
{
    if (!cmd || !g_variables || !g_context || !g_context->local_player)
        return;

    if (!g_variables->antiaim_enabled)
    {
        m_is_fake_angle_active = false;
        return;
    }

    // Update modes from config (packet manager integration)
    m_yaw_mode = static_cast<EAntiAimMode>(g_variables->antiaim_yaw_mode);
    m_pitch_mode = static_cast<EAntiAimPitch>(g_variables->antiaim_pitch_mode);
    m_fake_lag_ticks = g_variables->antiaim_fakelag_ticks;

    if (!g_context->local_player->is_alive())
        return;

    // Don't antiaim if noclip or ladder (movetype 8=noclip, 9=ladder)
    // TODO: reverse m_MoveType netvar, for now skip check
    /*
    __try {
        if (g_context->local_player->m_move_type() == 8 || g_context->local_player->m_move_type() == 9)
            return;
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
    */

    // Don't antiaim when using
    if (cmd->m_buttons & IN_USE)
        return;

    // Don't antiaim when throwing grenade (pin pulled)
    if (g_context->local_weapon)
    {
        __try {
            if (g_context->local_weapon->is_grenade() && cmd->m_buttons & IN_ATTACK)
            {
                // Check if pin pulled - simplified
                // In 2014, m_bPinPulled at 0x... - skip AA when grenade
                return;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    m_is_fake_angle_active = true;

    // Run pitch first (always)
    run_pitch(cmd);
    
    // Run yaw with packet manager integration
    run_yaw(cmd, send_packet);
    
    // LBY breaker for 2014: LBY updates every 1.1s when not moving, 0.22s when moving
    run_lby_breaker(cmd, send_packet);
}

void c_antiaim::run_pitch(CUserCmd* cmd)
{
    if (!cmd)
        return;

    switch (m_pitch_mode)
    {
    case EAntiAimPitch::DOWN:
        cmd->m_viewangles.pitch = 89.0f;
        break;
    case EAntiAimPitch::UP:
        cmd->m_viewangles.pitch = -89.0f;
        break;
    case EAntiAimPitch::ZERO_AA:
        cmd->m_viewangles.pitch = 0.0f;
        break;
    case EAntiAimPitch::JITTER_PITCH:
        cmd->m_viewangles.pitch = (m_jitter_side ? 89.0f : -89.0f);
        break;
    case EAntiAimPitch::NONE:
    default:
        break;
    }
}

void c_antiaim::run_yaw(CUserCmd* cmd, bool& send_packet)
{
    if (!cmd)
        return;

    QAngle original = cmd->m_viewangles;
    float max_desync = get_max_desync_delta();
    
    // Packet manager: get choked count
    int choked = 0;
    if (g_packet_manager)
        choked = g_packet_manager->get_choked_commands();

    QAngle real_angle = original;
    QAngle fake_angle = original;

    switch (m_yaw_mode)
    {
    case EAntiAimMode::BACKWARDS:
        real_angle.yaw = original.yaw + 180.0f;
        fake_angle.yaw = original.yaw;
        break;
        
    case EAntiAimMode::SIDEWAYS:
        real_angle.yaw = original.yaw + 90.0f;
        fake_angle.yaw = original.yaw - 90.0f;
        break;
        
    case EAntiAimMode::STATIC_180:
        // Classic 180 fake: real = original + 180, fake = original
        real_angle.yaw = original.yaw + 180.0f;
        fake_angle.yaw = original.yaw;
        // When choking, show fake, when sending, show real
        if (send_packet)
        {
            cmd->m_viewangles.yaw = real_angle.yaw;
            m_real_angle = real_angle;
        }
        else
        {
            cmd->m_viewangles.yaw = fake_angle.yaw;
            m_fake_angle = fake_angle;
        }
        return;
        
    case EAntiAimMode::JITTER:
        m_jitter_side = !m_jitter_side;
        if (m_jitter_side)
        {
            real_angle.yaw = original.yaw + 90.0f;
            fake_angle.yaw = original.yaw - 90.0f;
        }
        else
        {
            real_angle.yaw = original.yaw - 90.0f;
            fake_angle.yaw = original.yaw + 90.0f;
        }
        break;
        
    case EAntiAimMode::DESYNC_58:
    default:
        // Proper desync for 2014 build
        // Real = original, Fake = original + max_desync (58)
        // Logic: 
        // if bSendPacket -> real angle (what server sees)
        // if !bSendPacket -> fake angle (what enemies see when we choke)
        
        // In 2014, desync was done by:
        // - Choke 1 tick with fake angle
        // - Send next tick with real angle
        // This creates desync between client and server
        
        real_angle.yaw = original.yaw;
        fake_angle.yaw = original.yaw + max_desync;
        
        // Advanced: if we have packet manager and can choke, do 1 tick fake
        if (g_packet_manager)
        {
            if (choked < 1)
            {
                // First choked tick: fake angle
                cmd->m_viewangles.yaw = fake_angle.yaw;
                send_packet = false;
                m_fake_angle = fake_angle;
            }
            else
            {
                // Send real angle
                cmd->m_viewangles.yaw = real_angle.yaw;
                send_packet = true;
                m_real_angle = real_angle;
            }
        }
        else
        {
            // Fallback without packet manager: simple desync based on send_packet
            if (send_packet)
            {
                cmd->m_viewangles.yaw = real_angle.yaw;
                m_real_angle = real_angle;
            }
            else
            {
                cmd->m_viewangles.yaw = fake_angle.yaw;
                m_fake_angle = fake_angle;
            }
        }
        return;
    }

    // For other modes, apply based on send_packet
    if (send_packet)
    {
        cmd->m_viewangles.yaw = real_angle.yaw;
        m_real_angle = real_angle;
        m_last_real_angle = real_angle;
    }
    else
    {
        cmd->m_viewangles.yaw = fake_angle.yaw;
        m_fake_angle = fake_angle;
        m_last_fake_angle = fake_angle;
    }
}

void c_antiaim::run_lby_breaker(CUserCmd* cmd, bool& send_packet)
{
    if (!cmd || !g_interfaces || !g_interfaces->get_global_vars())
        return;

    // LBY Breaker for 2014: Lower Body Yaw updates every 1.1 sec when standing
    // We can break it by moving + fake lag
    // Reversed from server.dll: CCSPlayer::UpdateLBY
    
    float curtime = 0.0f;
    __try {
        curtime = g_interfaces->get_global_vars()->m_curtime;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return; }

    if (g_context && g_context->local_player)
    {
        // TODO: add m_vecVelocity netvar, for now assume 0
        float speed = 0.0f;
        /*
        Vector vel;
        __try { vel = g_context->local_player->m_velocity(); } __except(EXCEPTION_EXECUTE_HANDLER) { vel = Vector(0,0,0); }
        float speed = vel.Length2D();
        */
        
        // If moving, LBY updates constantly - no need to break
        if (speed > 0.1f)
        {
            m_next_lby_update = curtime + 0.22f;
            return;
        }

        // Standing: LBY updates every 1.1s
        // We want to choke and send fake angle just before LBY update
        if (curtime >= m_next_lby_update)
        {
            // Time to break LBY
            m_should_break_lby = true;
            m_next_lby_update = curtime + 1.1f;
            
            // Force fake angle that will become new LBY
            // In 2014, LBY = real yaw when moving or when breaking
            if (g_packet_manager)
            {
                // Choke 1 packet with 180 fake, then send
                if (g_packet_manager->get_choked_commands() < 1)
                {
                    cmd->m_viewangles.yaw += 180.0f;
                    send_packet = false;
                }
                else
                {
                    send_packet = true;
                }
            }
        }
        else
        {
            m_should_break_lby = false;
        }
    }
}

QAngle c_antiaim::get_real_angle()
{
    return m_real_angle;
}

QAngle c_antiaim::get_fake_angle()
{
    return m_fake_angle;
}

QAngle c_antiaim::get_real_angle(QAngle original)
{
    QAngle real = original;
    real.yaw += 0.0f;
    return real;
}

QAngle c_antiaim::get_fake_angle(QAngle original)
{
    QAngle fake = original;
    fake.yaw += get_max_desync_delta();
    return fake;
}
