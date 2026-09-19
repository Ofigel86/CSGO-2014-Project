#include "antiaim.hpp"
#include "../config/config.hpp"
#include "../interfaces/interfaces.hpp"
#include "../utilities/managers/packet_manager.hpp"
#include "../game/classes/entity.hpp"
#include <windows.h>

c_antiaim* g_antiaim = new c_antiaim();

c_antiaim::c_antiaim()
{
    m_yaw_mode = EAntiAimYaw::DESYNC;
    m_pitch_mode = EAntiAimPitch::DOWN;
}

c_antiaim::~c_antiaim()
{
}

void c_antiaim::instance(CUserCmd* cmd, bool& send_packet)
{
    if (!cmd || !g_variables || !g_context || !g_context->local_player)
        return;

    if (!g_variables->antiaim_enabled)
    {
        m_is_active = false;
        return;
    }

    if (!g_context->local_player->is_alive())
        return;

    __try {
        int mt = g_context->local_player->m_move_type();
        if (mt == 8 || mt == 9) // noclip, ladder
            return;
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    if (cmd->m_buttons & IN_USE)
        return;

    // Update from config - configurable jitter radius, default jitters at max speed
    m_yaw_mode = static_cast<EAntiAimYaw>(g_variables->antiaim_yaw_mode);
    m_pitch_mode = static_cast<EAntiAimPitch>(g_variables->antiaim_pitch_mode);
    m_jitter_range = static_cast<float>(g_variables->antiaim_jitter_range);
    m_jitter_range_pitch = static_cast<float>(g_variables->antiaim_jitter_range_pitch);
    m_jitter_random = g_variables->antiaim_jitter_random;
    m_jitter_speed = g_variables->antiaim_jitter_speed;
    if (m_jitter_speed < 1) m_jitter_speed = 1;

    m_is_active = true;

    run_pitch(cmd);
    run_yaw(cmd, send_packet);
}

void c_antiaim::run_pitch(CUserCmd* cmd)
{
    if (!cmd) return;

    // Jitter tick counter for max speed (every tick)
    m_jitter_tick++;
    bool should_jitter_pitch = (m_jitter_tick % m_jitter_speed) == 0;
    if (should_jitter_pitch)
        m_jitter_side_pitch = !m_jitter_side_pitch;

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
        // Configurable radius jitter at max speed
        // Default jitters: 89 / -89, but now radius configurable
        // e.g., range 10 = 89+10 / 89-10 or 0±range
        if (m_jitter_random)
        {
            // Random within radius
            float random_offset = 0.0f;
            __try {
                random_offset = static_cast<float>(rand() % (int)(m_jitter_range_pitch * 2 + 1)) - m_jitter_range_pitch;
            } __except(EXCEPTION_EXECUTE_HANDLER) { random_offset = m_jitter_range_pitch; }
            cmd->m_viewangles.pitch = (m_jitter_side_pitch ? 89.0f : -89.0f) + random_offset;
            // Clamp pitch to 89/-89
            if (cmd->m_viewangles.pitch > 89.0f) cmd->m_viewangles.pitch = 89.0f;
            if (cmd->m_viewangles.pitch < -89.0f) cmd->m_viewangles.pitch = -89.0f;
        }
        else
        {
            // Fixed jitter with configurable radius: e.g., 89±range
            // Max speed: switch every tick
            float base = 0.0f; // center at 0 for configurable, or 89 for default style
            // For configurable radius, we do: pitch = base ± range
            // If range=10, then jitter between -10 and +10, or 89±10
            // User wants radius configurable, default jitters at max speed
            // So: jitter between +range and -range around 0, or around 89
            // Let's do: if range < 89, jitter around 0 with radius, else around 89
            if (m_jitter_range_pitch >= 80.0f)
            {
                // Default style: 89 / -89 with radius as small offset
                cmd->m_viewangles.pitch = (m_jitter_side_pitch ? 89.0f : -89.0f);
                // Add radius as small variation
                if (m_jitter_range_pitch < 89.0f)
                    cmd->m_viewangles.pitch += (m_jitter_side_pitch ? m_jitter_range_pitch : -m_jitter_range_pitch) * 0.1f;
            }
            else
            {
                // Configurable radius around 0: e.g., range 30 = pitch jitter 30 / -30
                cmd->m_viewangles.pitch = (m_jitter_side_pitch ? m_jitter_range_pitch : -m_jitter_range_pitch);
            }
        }
        break;
    case EAntiAimPitch::NONE:
    default:
        break;
    }
}

void c_antiaim::run_yaw(CUserCmd* cmd, bool& send_packet)
{
    if (!cmd) return;

    QAngle original = cmd->m_viewangles;
    QAngle real = original;
    QAngle fake = original;

    // Configurable jitter radius at max speed
    // m_jitter_tick already incremented in run_pitch, but ensure
    // For yaw we also need max speed jitter every tick
    bool should_jitter_yaw = (m_jitter_tick % m_jitter_speed) == 0;
    if (should_jitter_yaw)
        m_jitter_side = !m_jitter_side;

    // Simple bSendPacket fake angle - 2014 style
    switch (m_yaw_mode)
    {
    case EAntiAimYaw::BACKWARDS:
        cmd->m_viewangles.yaw = original.yaw + 180.0f;
        m_real_angle = cmd->m_viewangles;
        m_fake_angle = cmd->m_viewangles;
        break;

    case EAntiAimYaw::SIDEWAYS:
        // Configurable radius: 90 becomes m_jitter_range
        if (m_jitter_side)
            cmd->m_viewangles.yaw = original.yaw + m_jitter_range;
        else
            cmd->m_viewangles.yaw = original.yaw - m_jitter_range;
        m_real_angle = cmd->m_viewangles;
        break;

    case EAntiAimYaw::STATIC_180:
        if (send_packet)
        {
            real.yaw = original.yaw + 180.0f;
            cmd->m_viewangles.yaw = real.yaw;
            m_real_angle = real;
        }
        else
        {
            fake.yaw = original.yaw;
            cmd->m_viewangles.yaw = fake.yaw;
            m_fake_angle = fake;
        }
        break;

    case EAntiAimYaw::JITTER:
        // Configurable radius jitter at MAX SPEED (every tick)
        // Default jitters: switch between +range and -range as fast as possible
        // User wants: radius configurable, jitters default at max speed
        if (m_jitter_random)
        {
            // Random jitter within radius
            float random_yaw = 0.0f;
            __try {
                // Random between -range and +range
                random_yaw = static_cast<float>(rand() % (int)(m_jitter_range * 2 + 1)) - m_jitter_range;
            } __except(EXCEPTION_EXECUTE_HANDLER) { random_yaw = m_jitter_range; }
            
            if (send_packet)
            {
                real.yaw = original.yaw + random_yaw;
                cmd->m_viewangles.yaw = real.yaw;
                m_real_angle = real;
            }
            else
            {
                fake.yaw = original.yaw - random_yaw;
                cmd->m_viewangles.yaw = fake.yaw;
                m_fake_angle = fake;
            }
        }
        else
        {
            // Fixed jitter: switch between +radius and -radius every tick (max speed)
            // e.g., range=45 => yaw = original +45 / original -45 each tick
            // For desync: real = +range, fake = -range
            float jitter_val = m_jitter_side ? m_jitter_range : -m_jitter_range;
            
            if (send_packet)
            {
                real.yaw = original.yaw + jitter_val;
                cmd->m_viewangles.yaw = real.yaw;
                m_real_angle = real;
            }
            else
            {
                fake.yaw = original.yaw - jitter_val;
                cmd->m_viewangles.yaw = fake.yaw;
                m_fake_angle = fake;
            }
        }
        break;

    case EAntiAimYaw::SPIN:
        // Spin speed also based on jitter range for configurability
        m_spin_yaw += m_jitter_range * 0.5f; // spin speed proportional to range
        if (m_spin_yaw > 180.0f) m_spin_yaw -= 360.0f;
        if (m_spin_yaw < -180.0f) m_spin_yaw += 360.0f;
        cmd->m_viewangles.yaw = original.yaw + m_spin_yaw;
        m_real_angle = cmd->m_viewangles;
        break;

    case EAntiAimYaw::DESYNC:
    default:
        // Desync with configurable radius: fake = original + range
        real.yaw = original.yaw;
        fake.yaw = original.yaw + m_jitter_range; // use configurable radius instead of fixed 180

        if (g_packet_manager)
        {
            int choked = g_packet_manager->get_choked_commands();
            if (choked < 1)
            {
                cmd->m_viewangles.yaw = fake.yaw;
                send_packet = false;
                m_fake_angle = fake;
            }
            else
            {
                cmd->m_viewangles.yaw = real.yaw;
                send_packet = true;
                m_real_angle = real;
            }
        }
        else
        {
            if (send_packet)
            {
                cmd->m_viewangles.yaw = real.yaw;
                m_real_angle = real;
            }
            else
            {
                cmd->m_viewangles.yaw = fake.yaw;
                m_fake_angle = fake;
            }
        }
        break;
    }
}
