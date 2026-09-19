#include "lag_comp.hpp"
#include "../interfaces/interfaces.hpp"
#include "../game/classes/entity.hpp"
#include "../utilities/context.hpp"
#include <windows.h>

c_lag_comp* g_lag_comp = new c_lag_comp();

c_lag_comp::c_lag_comp()
{
}

c_lag_comp::~c_lag_comp()
{
}

void c_lag_comp::instance()
{
    if (!g_interfaces || !g_context || !g_context->local_player)
        return;

    auto entity_list = g_interfaces->get_client_entity_list();
    auto global_vars = g_interfaces->get_global_vars();
    if (!entity_list || !global_vars)
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
            if (!player->is_alive()) {
                clear_records(i);
                continue;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) { continue; }

        // Get player data
        c_lag_record record;
        __try {
            record.m_origin = player->m_origin();
            record.m_velocity = player->m_velocity();
            record.m_flags = player->m_flags();
            record.m_duck_amount = player->m_duck_amount();
            record.m_simulation_time = player->m_simulation_time();
            // Eye angles from m_angEyeAngles[0] - need netvar, for now use abs?
            // In 2014, eye angles at m_angEyeAngles - try to get via animation?
            // Fallback: use origin + velocity dir
            record.m_tick_count = global_vars->m_tickcount;
            record.m_valid = true;
            
            // For jitter detection, store eye angles if we can get them
            // Try to get from player via m_angEyeAngles netvar (if reversed)
            // For now, approximate from velocity or store 0
            record.m_eye_angles = QAngle(0,0,0);
            
        } __except(EXCEPTION_EXECUTE_HANDLER) { continue; }

        // Check if valid tick (not too old)
        if (!is_valid_tick(record.m_simulation_time))
            continue;

        // Check if record already exists with same sim time
        auto latest = get_latest_record(i);
        if (latest && latest->m_simulation_time == record.m_simulation_time)
            continue;

        store_record(i, record);
    }
}

void c_lag_comp::store_record(int index, c_lag_record record)
{
    if (index < 0 || index >= 65) return;
    
    auto& records = m_records[index];
    
    // Detect jitter before storing
    if (!records.empty())
    {
        auto& last = records.front();
        __try {
            float delta = std::abs(record.m_eye_angles.yaw - last.m_eye_angles.yaw);
            if (delta > 180.0f) delta = 360.0f - delta;
            
            record.m_jitter_delta = delta;
            record.m_last_eye_angles = last.m_eye_angles;
            
            // If delta > 35 and switching quickly, it's jittering
            if (delta > 35.0f)
                record.m_is_jittering = true;
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
    
    records.push_front(record);
    
    // Keep max 12 records (2014 backtrack limit)
    while (records.size() > MAX_RECORDS)
        records.pop_back();
}

c_lag_record* c_lag_comp::get_latest_record(int index)
{
    if (index < 0 || index >= 65) return nullptr;
    auto& records = m_records[index];
    if (records.empty()) return nullptr;
    return &records.front();
}

c_lag_record* c_lag_comp::get_oldest_record(int index)
{
    if (index < 0 || index >= 65) return nullptr;
    auto& records = m_records[index];
    if (records.empty()) return nullptr;
    return &records.back();
}

std::deque<c_lag_record>* c_lag_comp::get_records(int index)
{
    if (index < 0 || index >= 65) return nullptr;
    return &m_records[index];
}

void c_lag_comp::clear_records(int index)
{
    if (index < 0 || index >= 65) return;
    m_records[index].clear();
}

void c_lag_comp::clear_all()
{
    for (int i = 0; i < 65; i++)
        m_records[i].clear();
}

bool c_lag_comp::is_valid_tick(float sim_time)
{
    if (!g_interfaces || !g_interfaces->get_global_vars())
        return false;

    auto global_vars = g_interfaces->get_global_vars();
    if (!global_vars) return false;

    __try {
        float curtime = global_vars->m_curtime;
        float delta = curtime - sim_time;
        // In 2014, valid if within 0.2 sec (200ms) for backtrack
        // sv_maxunlag = 0.2
        if (std::abs(delta) > 0.2f)
            return false;
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}
