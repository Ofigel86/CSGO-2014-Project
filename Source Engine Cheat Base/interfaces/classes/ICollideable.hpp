#pragma once
#include "../../math/Vector.hpp"

enum SolidType_t
{
    SOLID_NONE = 0,
    SOLID_BSP = 1,
    SOLID_BBOX = 2,
    SOLID_OBB = 3,
    SOLID_OBB_YAW = 4,
    SOLID_CUSTOM = 5,
    SOLID_VPHYSICS = 6,
    SOLID_LAST,
};

class ICollideable
{
public:
    virtual void* GetEntityHandle() = 0;
    virtual const Vector& OBBMins() const = 0;
    virtual const Vector& OBBMaxs() const = 0;
};
