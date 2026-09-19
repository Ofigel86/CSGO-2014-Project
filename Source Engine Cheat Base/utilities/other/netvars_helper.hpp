#pragma once
#include "../../interfaces/interfaces.hpp"

#define NETVAR(type, name, table, netvar)                           \
    type& name##() const {                                          \
        static int _##name = netvars::get().get_offset(table, netvar);     \
        return *(type*)((uintptr_t)this + _##name);                 \
        }

#define PNETVAR(type, name, table, netvar)                           \
    type* name##() const {                                          \
        static int _##name = netvars::get().get_offset(table, netvar);     \
        return (type*)((uintptr_t)this + _##name);                 \
        }

#define OFFSET(type, name, offset)\
type &name##() const\
{\
        return *(type*)(uintptr_t(this) + offset);\
}