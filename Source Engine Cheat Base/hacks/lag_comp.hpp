#pragma once
#include "../math/Vector.hpp"
#include "../math/QAngle.hpp"
#include <deque>
#include <array>

// Lag record for 2014 CS:GO - stores player history for backtrack and resolver
// Reversed from server.dll: CBasePlayer lag compensation stores origin, angles, simulation time

struct c_lag_record
{
    c_lag_record() = default;
    
    Vector m_origin = Vector(0,0,0);
    Vector m_velocity = Vector(0,0,0);
    QAngle m_eye_angles = QAngle(0,0,0);
    QAngle m_abs_angles = QAngle(0,0,0);
    float m_simulation_time = 0.0f;
    int m_flags = 0;
    float m_duck_amount = 0.0f;
    int m_tick_count = 0;
    
    // For resolver
    bool m_is_jittering = false;
    float m_jitter_delta = 0.0f;
    QAngle m_last_eye_angles = QAngle(0,0,0);
    
    bool m_valid = false;
};

class c_lag_comp
{
public:
    c_lag_comp();
    ~c_lag_comp();

    void instance();
    void store_record(int index, c_lag_record record);
    c_lag_record* get_latest_record(int index);
    c_lag_record* get_oldest_record(int index);
    std::deque<c_lag_record>* get_records(int index);
    
    void clear_records(int index);
    void clear_all();

    // Backtrack
    bool is_valid_tick(float sim_time);
    
    // For resolver - get history
    static const int MAX_RECORDS = 12; // 2014 max, 12 ticks backtrack
    std::array<std::deque<c_lag_record>, 65> m_records;
};

extern c_lag_comp* g_lag_comp;
