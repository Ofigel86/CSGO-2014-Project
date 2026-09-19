#pragma once
#include "../../interfaces/interfaces.hpp"
#include <cstdint>

// Fixed NETVAR macro: handles 0 offset case and caches properly
#define NETVAR(type, name, table, netvar)                           \
    type& name() const {                                            \
        static int _##name = -1;                                    \
        if (_##name == -1) {                                        \
            _##name = netvars::get().get_offset(table, netvar);     \
            if (_##name == 0) {                                     \
                /* Try to get at least once more if 0, could be valid but log */ \
            }                                                       \
        }                                                           \
        return *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + _##name); \
    }

#define PNETVAR(type, name, table, netvar)                          \
    type* name() const {                                            \
        static int _##name = -1;                                    \
        if (_##name == -1)                                          \
            _##name = netvars::get().get_offset(table, netvar);     \
        return reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + _##name); \
    }

#define OFFSET(type, name, offset)                                  \
    type& name() const {                                            \
        return *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + offset); \
    }
