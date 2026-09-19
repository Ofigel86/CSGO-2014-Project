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

    // Update from config
    m_yaw_mode = static_cast<EAntiAimYaw>(g_variables->antiaim_yaw_mode);
    m_pitch_mode = static_cast<EAntiAimPitch>(g_variables->antiaim_pitch_mode);

    m_is_active = true;

    run_pitch(cmd);
    run_yaw(cmd, send_packet);
}

void c_antiaim::run_pitch(CUserCmd* cmd)
{
    if (!cmd) return;

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
    if (!cmd) return;

    QAngle original = cmd->m_viewangles;
    QAngle real = original;
    QAngle fake = original;

    // Simple bSendPacket fake angle - 2014 style
    // When send_packet = true -> server receives angle (real)
    // When send_packet = false -> packet choked, enemies see previous or fake

    switch (m_yaw_mode)
    {
    case EAntiAimYaw::BACKWARDS:
        // Both real and fake 180 - simple
        cmd->m_viewangles.yaw = original.yaw + 180.0f;
        m_real_angle = cmd->m_viewangles;
        m_fake_angle = cmd->m_viewangles;
        break;

    case EAntiAimYaw::SIDEWAYS:
        m_jitter_side = !m_jitter_side;
        if (m_jitter_side)
            cmd->m_viewangles.yaw = original.yaw + 90.0f;
        else
            cmd->m_viewangles.yaw = original.yaw - 90.0f;
        m_real_angle = cmd->m_viewangles;
        break;

    case EAntiAimYaw::STATIC_180:
        // Classic 2014: real = 180, fake = 0
        // Use packet manager to decide
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
        m_jitter_side = !m_jitter_side;
        if (send_packet)
        {
            // Real angle
            if (m_jitter_side)
                real.yaw = original.yaw + 90.0f;
            else
                real.yaw = original.yaw - 90.0f;
            cmd->m_viewangles.yaw = real.yaw;
            m_real_angle = real;
        }
        else
        {
            // Fake angle opposite
            if (m_jitter_side)
                fake.yaw = original.yaw - 90.0f;
            else
                fake.yaw = original.yaw + 90.0f;
            cmd->m_viewangles.yaw = fake.yaw;
            m_fake_angle = fake;
        }
        break;

    case EAntiAimYaw::SPIN:
        m_spin_yaw += 45.0f;
        if (m_spin_yaw > 180.0f) m_spin_yaw -= 360.0f;
        cmd->m_viewangles.yaw = original.yaw + m_spin_yaw;
        m_real_angle = cmd->m_viewangles;
        break;

    case EAntiAimYaw::DESYNC:
    default:
        // Simple desync for 2014 - no LBY, just bSendPacket
        // Real = original, Fake = original + 180 (or +90)
        // In 2014 you could do 180 because no LBY clamp
        real.yaw = original.yaw;
        fake.yaw = original.yaw + 180.0f; // 180 fake, classic

        // If packet manager says we can choke, do fake
        if (g_packet_manager)
        {
            int choked = g_packet_manager->get_choked_commands();
            if (choked < 1)
            {
                // Choke fake
                cmd->m_viewangles.yaw = fake.yaw;
                send_packet = false;
                m_fake_angle = fake;
            }
            else
            {
                // Send real
                cmd->m_viewangles.yaw = real.yaw;
                send_packet = true;
                m_real_angle = real;
            }
        }
        else
        {
            // Fallback
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
