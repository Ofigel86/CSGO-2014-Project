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

    // 2014 limit: max fake = 90 deg total. If set >90, real moves away!
    // So clamp jitter_range to 90 max for desync, and to 45 for jitter both sides (diff 90)
    float clamped_range = m_jitter_range;
    if (clamped_range > MAX_DESYNC_2014) clamped_range = MAX_DESYNC_2014;
    if (clamped_range < -MAX_DESYNC_2014) clamped_range = -MAX_DESYNC_2014;

    // For jitter mode where real=+range fake=-range, total diff = 2*range must be <=90
    // So half range max = 45
    float jitter_half = m_jitter_range;
    if (jitter_half > MAX_JITTER_HALF_2014) jitter_half = MAX_JITTER_HALF_2014;
    if (jitter_half < -MAX_JITTER_HALF_2014) jitter_half = -MAX_JITTER_HALF_2014;

    // Configurable jitter radius at max speed
    bool should_jitter_yaw = (m_jitter_tick % m_jitter_speed) == 0;
    if (should_jitter_yaw)
        m_jitter_side = !m_jitter_side;

    // Simple bSendPacket fake angle - 2014 style with 90 max
    switch (m_yaw_mode)
    {
    case EAntiAimYaw::BACKWARDS:
        // In 2014 backwards 180 would move real! Max is 90.
        // So backwards = original + 90 (max desync) not 180
        cmd->m_viewangles.yaw = original.yaw + clamped_range; // clamped to 90
        // If user set range 45, backwards = +45, but for true backwards use 90
        // Override to 90 for backwards mode to maximize
        if (fabs(m_jitter_range) < 80.0f) // if user left default 45, use 90 for backwards
            cmd->m_viewangles.yaw = original.yaw + MAX_DESYNC_2014;
        m_real_angle = cmd->m_viewangles;
        m_fake_angle = cmd->m_viewangles;
        break;

    case EAntiAimYaw::SIDEWAYS:
        // Sideways = 90 / -90 max, use clamped_range
        if (m_jitter_side)
            cmd->m_viewangles.yaw = original.yaw + clamped_range;
        else
            cmd->m_viewangles.yaw = original.yaw - clamped_range;
        m_real_angle = cmd->m_viewangles;
        break;

    case EAntiAimYaw::STATIC_180:
        // STATIC_180 in 2014 impossible with 90 limit: 180 would move real
        // So we do: real = original, fake = original + 90 = max desync
        // Or real = original+90, fake = original = 90 desync
        // Implement as: real = original, fake = original + 90 (max)
        real.yaw = original.yaw;
        fake.yaw = original.yaw + MAX_DESYNC_2014; // 90 max, not 180!

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
        break;

    case EAntiAimYaw::JITTER:
        // Jitter with 90 max total diff
        // If real=+range fake=-range, diff=2*range must be <=90, so range<=45
        // Use jitter_half = clamp(range, 45)
        if (m_jitter_random)
        {
            // Random jitter within half range (so total diff <=90)
            float random_yaw = 0.0f;
            __try {
                // Random between -half and +half
                float range = fabs(jitter_half);
                if (range < 1.0f) range = MAX_JITTER_HALF_2014;
                random_yaw = static_cast<float>(rand() % (int)(range * 2 + 1)) - range;
            } __except(EXCEPTION_EXECUTE_HANDLER) { random_yaw = jitter_half; }
            
            if (send_packet)
            {
                real.yaw = original.yaw + random_yaw;
                cmd->m_viewangles.yaw = real.yaw;
                m_real_angle = real;
            }
            else
            {
                fake.yaw = original.yaw - random_yaw; // opposite side, diff = 2*random <=90
                cmd->m_viewangles.yaw = fake.yaw;
                m_fake_angle = fake;
            }
        }
        else
        {
            // Fixed jitter: switch between +half and -half every tick (max speed)
            // diff = 2*half <=90
            float jitter_val = m_jitter_side ? jitter_half : -jitter_half;
            
            if (send_packet)
            {
                real.yaw = original.yaw + jitter_val;
                cmd->m_viewangles.yaw = real.yaw;
                m_real_angle = real;
            }
            else
            {
                fake.yaw = original.yaw - jitter_val; // opposite, total 90 max
                cmd->m_viewangles.yaw = fake.yaw;
                m_fake_angle = fake;
            }
        }
        break;

    case EAntiAimYaw::SPIN:
        m_spin_yaw += clamped_range * 0.5f;
        if (m_spin_yaw > 180.0f) m_spin_yaw -= 360.0f;
        if (m_spin_yaw < -180.0f) m_spin_yaw += 360.0f;

        // For spin with desync: real = spin, fake = spin + 90 max
        real.yaw = original.yaw + m_spin_yaw;
        fake.yaw = real.yaw + MAX_DESYNC_2014;

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
        break;

    case EAntiAimYaw::DESYNC:
    default:
        // Desync: real = original, fake = original + clamped_range (max 90)
        // This is proper 2014 desync: 90 max, if set more real moves away
        real.yaw = original.yaw;
        fake.yaw = original.yaw + clamped_range; // max 90

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
