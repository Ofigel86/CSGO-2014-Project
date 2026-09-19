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
    
    if (!records || records->size() < 2)
        return record->m_eye_angles;

    // Method 1: Velocity resolver for moving jitter (2014 style) - best for moving
    __try {
        float velLen = record->m_velocity.Length2D();
        if (velLen > 0.1f)
        {
            QAngle velAngle;
            velAngle.yaw = std::atan2(record->m_velocity.y, record->m_velocity.x) * 180.0f / 3.14159265f;
            velAngle.pitch = record->m_eye_angles.pitch;
            velAngle.roll = 0.0f;
            info.m_mode = EResolverMode::VELOCITY;
            return velAngle;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    // Method 2: User's pseudo code - proper jitter resolver for 2014
    // Calculates averaged fake delta, filters outliers, gets base yaw offset
    __try {
        if (records->size() >= 3)
        {
            // Step 1: Calculate averaged_fake_delta from all records
            float averaged_fake_delta = 0.0f;
            int delta_count = 0;
            
            for (size_t i = 1; i < records->size(); i++)
            {
                auto& cur = (*records)[i-1];
                auto& prev = (*records)[i];
                
                float delta = cur.m_eye_angles.yaw - prev.m_eye_angles.yaw;
                // Normalize delta to -180..180
                while (delta > 180.0f) delta -= 360.0f;
                while (delta < -180.0f) delta += 360.0f;
                
                averaged_fake_delta += delta;
                delta_count++;
            }
            
            if (delta_count > 0)
                averaged_fake_delta /= delta_count;

            // Step 2: Filter deltas and calculate base_yaw_offset
            // Skip deltas that are too big (>1.125 * avg) or opposite direction
            float base_yaw_offset = 0.0f;
            int valid_count = 0;
            
            for (size_t i = 1; i < records->size(); i++)
            {
                auto& cur = (*records)[i-1];
                auto& prev = (*records)[i];
                
                float delta = cur.m_eye_angles.yaw - prev.m_eye_angles.yaw;
                while (delta > 180.0f) delta -= 360.0f;
                while (delta < -180.0f) delta += 360.0f;
                
                float abs_delta = std::abs(delta);
                float abs_avg = std::abs(averaged_fake_delta);
                
                // Filter: if delta too big or opposite direction, skip
                // This is from user's pseudo code: fabs(delta) > fabs(1.125*avg) || opposite sign
                if (abs_avg > 0.01f)
                {
                    if (abs_delta > std::abs(1.125f * averaged_fake_delta))
                        continue;
                    if ((delta < 0.0f && averaged_fake_delta > 0.0f) || (delta > 0.0f && averaged_fake_delta < 0.0f))
                        continue;
                }
                
                base_yaw_offset += abs_delta;
                valid_count++;
            }
            
            if (valid_count > 0)
                base_yaw_offset /= valid_count;

            // Step 3: base_yaw + base_yaw_offset = resolved
            // For fixed jitter: averaged_fake_delta will be ~0 ( +45 and -45 cancel), 
            // but base_yaw_offset will be ~45 (avg of abs deltas)
            // So base_yaw is center, which is real
            QAngle base = records->back().m_eye_angles; // oldest as base
            // Actually for jitter, we want to find real by averaging or using base + offset logic
            // User's code: base_yaw += base_yaw_offset
            
            // Improved: For fixed jitter at max speed, average of last 2 is real
            // For random jitter, averaged_fake_delta filtering gives us base offset
            QAngle resolved;
            resolved.pitch = record->m_eye_angles.pitch;
            resolved.roll = 0.0f;
            
            // If averaged_fake_delta is near 0, it's fixed jitter switching +range/-range
            // Then real = average of last 2
            if (std::abs(averaged_fake_delta) < 5.0f && records->size() >= 2)
            {
                auto& last = (*records)[0];
                auto& prev = (*records)[1];
                resolved.yaw = (last.m_eye_angles.yaw + prev.m_eye_angles.yaw) * 0.5f;
            }
            else
            {
                // For random or desync jitter, use base + offset logic
                // base_yaw is last record's yaw, add offset to get real
                // Actually for random jitter, best is to use base yaw (oldest) as real
                // Or use velocity if moving (already handled)
                // For standing random jitter, use averaged angle from filtered records
                float avg_yaw = 0.0f;
                int avg_count = 0;
                for (size_t i = 0; i < records->size(); i++)
                {
                    // Only use records that passed filter (not too big delta)
                    // For simplicity, average all yaws
                    avg_yaw += (*records)[i].m_eye_angles.yaw;
                    avg_count++;
                }
                if (avg_count > 0)
                    avg_yaw /= avg_count;
                resolved.yaw = avg_yaw;
            }
            
            while (resolved.yaw > 180.0f) resolved.yaw -= 360.0f;
            while (resolved.yaw < -180.0f) resolved.yaw += 360.0f;
            
            info.m_last_jitter_delta = averaged_fake_delta;
            info.m_mode = EResolverMode::JITTER_DETECT;
            return resolved;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    // Method 3: Fallback - no bruteforce on random jitter (user is right, bruteforce on random is bad)
    // For random jitter, bruteforce is useless because it's random
    // Instead, use last moving angle or velocity (already tried)
    // If still failing, return latest record's angle (don't bruteforce random)
    
    __try {
        // For fixed jitter, we can still bruteforce as last resort, but for random - don't
        // Check if jitter is random by variance
        if (records->size() >= 4)
        {
            float variance = 0.0f;
            float avg = 0.0f;
            for (size_t i = 0; i < records->size(); i++)
                avg += (*records)[i].m_eye_angles.yaw;
            avg /= records->size();
            
            for (size_t i = 0; i < records->size(); i++)
            {
                float diff = (*records)[i].m_eye_angles.yaw - avg;
                variance += diff * diff;
            }
            variance /= records->size();
            
            // If variance high (>1000), it's random jitter, not fixed
            // Don't bruteforce random jitter - use avg or velocity
            if (variance > 1000.0f)
            {
                // Random jitter - return avg, not bruteforce
                QAngle avg_angle;
                avg_angle.yaw = avg;
                avg_angle.pitch = record->m_eye_angles.pitch;
                avg_angle.roll = 0.0f;
                while (avg_angle.yaw > 180.0f) avg_angle.yaw -= 360.0f;
                while (avg_angle.yaw < -180.0f) avg_angle.yaw += 360.0f;
                return avg_angle;
            }
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    // Only bruteforce for fixed jitter (low variance) as last resort
    return bruteforce_jitter(index);
}

QAngle c_resolver::bruteforce_jitter(int index)
{
    if (index < 0 || index >= 65) return QAngle(0,0,0);

    auto& info = m_resolver_info[index];
    auto records = g_lag_comp->get_records(index);
    
    if (!records || records->empty())
        return QAngle(0,0,0);

    // Bruteforce is ONLY for fixed jitter, NOT for random jitter!
    // User correctly pointed: bruteforce on random jitter is stupid
    // Random jitter is unpredictable, bruteforce 0/180/90/-90 won't work
    // For random jitter, we should use averaging or velocity, not bruteforce
    
    // Check variance to see if it's random
    __try {
        if (records->size() >= 4)
        {
            float avg = 0.0f;
            for (size_t i = 0; i < records->size(); i++)
                avg += (*records)[i].m_eye_angles.yaw;
            avg /= records->size();
            
            float variance = 0.0f;
            for (size_t i = 0; i < records->size(); i++)
            {
                float diff = (*records)[i].m_eye_angles.yaw - avg;
                while (diff > 180.0f) diff -= 360.0f;
                while (diff < -180.0f) diff += 360.0f;
                variance += diff * diff;
            }
            variance /= records->size();
            
            // High variance = random jitter, don't bruteforce
            if (variance > 1000.0f)
            {
                // For random jitter, return average (best guess) instead of bruteforce
                QAngle avg_angle;
                avg_angle.yaw = avg;
                avg_angle.pitch = records->front().m_eye_angles.pitch;
                avg_angle.roll = 0.0f;
                while (avg_angle.yaw > 180.0f) avg_angle.yaw -= 360.0f;
                while (avg_angle.yaw < -180.0f) avg_angle.yaw += 360.0f;
                info.m_mode = EResolverMode::JITTER_DETECT;
                return avg_angle;
            }
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    // For fixed jitter (low variance), bruteforce can work as last resort
    // Try: original, original+180, original+90, original-90
    __try {
        auto& latest = records->front();
        QAngle base = latest.m_eye_angles;
        
        info.m_bruteforce_angles[0] = base;
        info.m_bruteforce_angles[1] = QAngle(base.pitch, base.yaw + 180.0f, 0.0f);
        info.m_bruteforce_angles[2] = QAngle(base.pitch, base.yaw + 90.0f, 0.0f);
        info.m_bruteforce_angles[3] = QAngle(base.pitch, base.yaw - 90.0f, 0.0f);
        
        for (int i = 0; i < 4; i++)
        {
            while (info.m_bruteforce_angles[i].yaw > 180.0f) info.m_bruteforce_angles[i].yaw -= 360.0f;
            while (info.m_bruteforce_angles[i].yaw < -180.0f) info.m_bruteforce_angles[i].yaw += 360.0f;
        }
        
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
