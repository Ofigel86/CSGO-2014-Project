#pragma once
#include "../math/QAngle.hpp"
#include "../math/Vector.hpp"
#include "lag_comp.hpp"
#include <array>

// Resolver against jitters using lag records (2014 CS:GO, no LBY)
// User thought we made resolver on lag records against jitters - now we do it for real

enum class EResolverMode
{
    NONE = 0,
    JITTER_DETECT,      // Detect jitter pattern from lag records
    BRUTEFORCE,         // Bruteforce real/fake angles
    VELOCITY,           // Use velocity dir for moving jitter
    LAST_MOVING,        // Use last moving angle
    STATIC_DESYNC       // Resolve desync (real vs fake)
};

struct c_resolver_info
{
    bool m_is_jittering = false;
    bool m_is_moving = false;
    float m_last_jitter_delta = 0.0f;
    QAngle m_last_eye_angles = QAngle(0,0,0);
    QAngle m_resolved_angles = QAngle(0,0,0);
    QAngle m_original_angles = QAngle(0,0,0);
    int m_jitter_ticks = 0;
    int m_bruteforce_index = 0;
    EResolverMode m_mode = EResolverMode::NONE;
    float m_last_resolve_time = 0.0f;
    
    // For bruteforce
    std::array<QAngle, 4> m_bruteforce_angles;
    int m_current_brute = 0;
};

class c_resolver
{
public:
    c_resolver();
    ~c_resolver();

    void instance();
    void resolve_player(int index, c_lag_record* record);
    
    // Jitter detection from lag records
    bool is_jittering(c_lag_record* current, c_lag_record* previous);
    bool is_jittering_from_history(int index);
    
    // Resolver methods
    QAngle resolve_jitter(int index, c_lag_record* record);
    QAngle resolve_desync(int index, c_lag_record* record);
    QAngle bruteforce_jitter(int index);
    
    // Get resolved angle
    QAngle get_resolved_angle(int index);
    
    std::array<c_resolver_info, 65> m_resolver_info;
};

extern c_resolver* g_resolver;
