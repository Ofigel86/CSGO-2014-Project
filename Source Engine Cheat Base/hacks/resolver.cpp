#include "resolver.hpp"
#include "lag_comp.hpp"
#include "../interfaces/interfaces.hpp"
#include "../game/classes/entity.hpp"
#include "../utilities/context.hpp"
#include <windows.h>
#include <cmath>

c_resolver* g_resolver = new c_resolver();

c_resolver::c_resolver()
{
}

c_resolver::~c_resolver()
{
}

void c_resolver::instance()
{
    if (!g_lag_comp || !g_context || !g_context->local_player)
        return;

    auto entity_list = g_interfaces->get_client_entity_list();
    if (!entity_list)
        return;

    for (int i = 1; i < 65; i++)
    {
        c_cs_player* player = nullptr;
        __try {
            player = reinterpret_cast<c_cs_player*>(entity_list->GetClientEntity(i));
        } __except(EXCEPTION_EXECUTE_HANDLER) { continue; }

        if (!player) continue;
        if (player == g_context->local_player) continue;

        __try {
            if (!player->is_alive()) continue;
        } __except(EXCEPTION_EXECUTE_HANDLER) { continue; }

        auto latest = g_lag_comp->get_latest_record(i);
        if (!latest) continue;

        resolve_player(i, latest);
    }
}

void c_resolver::resolve_player(int index, c_lag_record* record)
{
    if (index < 0 || index >= 65 || !record) return;

    auto& info = m_resolver_info[index];
    info.m_original_angles = record->m_eye_angles;

    // Check if jittering from history
    bool jittering = is_jittering_from_history(index);
    info.m_is_jittering = jittering;

    if (jittering)
    {
        info.m_jitter_ticks++;
        info.m_mode = EResolverMode::JITTER_DETECT;
        
        // Resolve jitter
        QAngle resolved = resolve_jitter(index, record);
        info.m_resolved_angles = resolved;
        record->m_eye_angles = resolved; // Override for aimbot
    }
    else
    {
        info.m_jitter_ticks = 0;
        info.m_resolved_angles = record->m_eye_angles;
    }

    // Check moving
    __try {
        float velLen = record->m_velocity.Length2D();
        info.m_is_moving = velLen > 0.1f;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        info.m_is_moving = false;
    }
}

bool c_resolver::is_jittering(c_lag_record* current, c_lag_record* previous)
{
    if (!current || !previous) return false;

    __try {
        float delta = std::abs(current->m_eye_angles.yaw - previous->m_eye_angles.yaw);
        if (delta > 180.0f) delta = 360.0f - delta;
        
        // In 2014, jitter is typically >35° switching quickly
        // Fixed jitter: +45/-45 every tick (max speed)
        // Random jitter: random within radius
        if (delta > 35.0f)
            return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    return false;
}

bool c_resolver::is_jittering_from_history(int index)
{
    if (index < 0 || index >= 65) return false;
    
    auto records = g_lag_comp->get_records(index);
    if (!records || records->size() < 3) return false;

    // Check last 3 records for jitter pattern
    // If yaw is switching back and forth rapidly, it's jittering
    int jitter_count = 0;
    
    __try {
        for (size_t i = 1; i < records->size() && i < 4; i++)
        {
            auto& current = (*records)[i-1];
            auto& prev = (*records)[i];
            
            float delta = std::abs(current.m_eye_angles.yaw - prev.m_eye_angles.yaw);
            if (delta > 180.0f) delta = 360.0f - delta;
            
            if (delta > 35.0f)
                jitter_count++;
        }
        
        // If 2+ of last 3 ticks have big delta, it's jittering
        if (jitter_count >= 2)
            return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    return false;
}

QAngle c_resolver::resolve_jitter(int index, c_lag_record* record)
{
    if (index < 0 || index >= 65 || !record) return QAngle(0,0,0);

    auto& info = m_resolver_info[index];
    auto records = g_lag_comp->get_records(index);
    
    if (!records || records->empty())
        return record->m_eye_angles;

    // Method 1: Velocity resolver for moving jitter (2014 style)
    // If moving, real angle = velocity direction
    __try {
        float velLen = record->m_velocity.Length2D();
        if (velLen > 0.1f)
        {
            // Use velocity yaw as real
            QAngle velAngle;
            velAngle.yaw = std::atan2(record->m_velocity.y, record->m_velocity.x) * 180.0f / 3.14159265f;
            velAngle.pitch = 0.0f;
            velAngle.roll = 0.0f;
            info.m_mode = EResolverMode::VELOCITY;
            return velAngle;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    // Method 2: For standing jitter, use bruteforce or last moving
    // In 2014, jitter is often fixed +range/-range at max speed
    // We can detect radius from history and resolve to one side
    
    __try {
        // Calculate average jitter radius from history
        float total_delta = 0.0f;
        int count = 0;
        for (size_t i = 1; i < records->size() && i < 5; i++)
        {
            auto& cur = (*records)[i-1];
            auto& prev = (*records)[i];
            float delta = std::abs(cur.m_eye_angles.yaw - prev.m_eye_angles.yaw);
            if (delta > 180.0f) delta = 360.0f - delta;
            if (delta > 10.0f) // ignore small movements
            {
                total_delta += delta;
                count++;
            }
        }
        
        float avg_jitter = count > 0 ? total_delta / count : 45.0f;
        info.m_last_jitter_delta = avg_jitter;

        // If we have jitter history, try to resolve to center or to one side
        // For fixed jitter: angles are +range and -range, center is real
        // So we can average last 2
        if (records->size() >= 2)
        {
            auto& last = (*records)[0];
            auto& prev = (*records)[1];
            
            // Average = center (real) for fixed jitter
            QAngle averaged;
            averaged.yaw = (last.m_eye_angles.yaw + prev.m_eye_angles.yaw) * 0.5f;
            averaged.pitch = last.m_eye_angles.pitch;
            averaged.roll = 0.0f;
            
            // Normalize
            while (averaged.yaw > 180.0f) averaged.yaw -= 360.0f;
            while (averaged.yaw < -180.0f) averaged.yaw += 360.0f;
            
            info.m_mode = EResolverMode::JITTER_DETECT;
            return averaged;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    // Method 3: Bruteforce if other methods fail
    return bruteforce_jitter(index);
}

QAngle c_resolver::bruteforce_jitter(int index)
{
    if (index < 0 || index >= 65) return QAngle(0,0,0);

    auto& info = m_resolver_info[index];
    auto records = g_lag_comp->get_records(index);
    
    if (!records || records->empty())
        return QAngle(0,0,0);

    // Bruteforce angles for 2014 jitter:
    // Try: original, original+180, original+90, original-90
    // Cycle through them each shot
    
    __try {
        auto& latest = records->front();
        QAngle base = latest.m_eye_angles;
        
        // Setup bruteforce angles
        info.m_bruteforce_angles[0] = base; // original
        info.m_bruteforce_angles[1] = QAngle(base.pitch, base.yaw + 180.0f, 0.0f); // 180
        info.m_bruteforce_angles[2] = QAngle(base.pitch, base.yaw + 90.0f, 0.0f); // 90
        info.m_bruteforce_angles[3] = QAngle(base.pitch, base.yaw - 90.0f, 0.0f); // -90
        
        // Normalize
        for (int i = 0; i < 4; i++)
        {
            while (info.m_bruteforce_angles[i].yaw > 180.0f) info.m_bruteforce_angles[i].yaw -= 360.0f;
            while (info.m_bruteforce_angles[i].yaw < -180.0f) info.m_bruteforce_angles[i].yaw += 360.0f;
        }
        
        // Cycle
        QAngle result = info.m_bruteforce_angles[info.m_current_brute % 4];
        info.m_current_brute++;
        info.m_mode = EResolverMode::BRUTEFORCE;
        
        return result;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return QAngle(0,0,0);
    }
}

QAngle c_resolver::resolve_desync(int index, c_lag_record* record)
{
    // For desync (real vs fake), in 2014 without LBY it's simpler
    // Real = moving direction or last moving angle
    // Fake = desync angle (real + 180 or +58)
    
    if (!record) return QAngle(0,0,0);
    
    auto& info = m_resolver_info[index];
    
    __try {
        // If moving, desync is small, use velocity
        float velLen = record->m_velocity.Length2D();
        if (velLen > 0.1f)
        {
            QAngle velAngle;
            velAngle.yaw = std::atan2(record->m_velocity.y, record->m_velocity.x) * 180.0f / 3.14159265f;
            return velAngle;
        }
        
        // Standing: try to resolve desync by checking if eye angles are 180 from last moving
        // For now, return original
        return record->m_eye_angles;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return record->m_eye_angles;
    }
}

QAngle c_resolver::get_resolved_angle(int index)
{
    if (index < 0 || index >= 65) return QAngle(0,0,0);
    return m_resolver_info[index].m_resolved_angles;
}
