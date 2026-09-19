#pragma once
#include "../../math/Vector.hpp"
#include "../../math/Vector4D.hpp"
#include "../../math/QAngle.hpp"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEG2RAD( x ) ( ( float )( x ) * ( float )( ( float )( M_PI ) / 180.0f ) )
#define RAD2DEG( x ) ( ( float )( x ) * ( float )( 180.0f / ( float )( M_PI ) ) )

class c_math
{
public:
    void sin_cos(float a, float* s, float* c)
    {
        if (!s || !c)
            return;
        *s = sinf(a);
        *c = cosf(a);
    }

    void angle_vectors(const QAngle& angles, Vector& forward, Vector& right, Vector& up)
    {
        float sr, sp, sy, cr, cp, cy;

        sin_cos(DEG2RAD(angles[1]), &sy, &cy);
        sin_cos(DEG2RAD(angles[0]), &sp, &cp);
        sin_cos(DEG2RAD(angles[2]), &sr, &cr);

        forward.x = (cp * cy);
        forward.y = (cp * sy);
        forward.z = (-sp);
        right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
        right.y = (-1 * sr * sp * sy + -1 * cr * cy);
        right.z = (-1 * sr * cp);
        up.x = (cr * sp * cy + -sr * -sy);
        up.y = (cr * sp * sy + -sr * cy);
        up.z = (cr * cp);
    }

    void angle_vectors(const QAngle& angles, Vector& forward)
    {
        float sp, sy, cp, cy;

        sin_cos(DEG2RAD(angles[1]), &sy, &cy);
        sin_cos(DEG2RAD(angles[0]), &sp, &cp);

        forward.x = cp * cy;
        forward.y = cp * sy;
        forward.z = -sp;

        forward.Normalize();
    }

    Vector cross_product(const Vector& a, const Vector& b)
    {
        return Vector(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
    }

    void vector_angles(const Vector& forward, Vector& up, QAngle& angles)
    {
        Vector left = cross_product(up, forward);
        left.NormalizeInPlace();

        float forwardDist = forward.Length2D();

        if (forwardDist > 0.001f)
        {
            angles.pitch = atan2f(-forward.z, forwardDist) * 180.0f / static_cast<float>(M_PI);
            angles.yaw = atan2f(forward.y, forward.x) * 180.0f / static_cast<float>(M_PI);

            float upZ = (left.y * forward.x) - (left.x * forward.y);
            angles.roll = atan2f(left.z, upZ) * 180.0f / static_cast<float>(M_PI);
        }
        else
        {
            angles.pitch = atan2f(-forward.z, forwardDist) * 180.0f / static_cast<float>(M_PI);
            angles.yaw = atan2f(-left.x, left.y) * 180.0f / static_cast<float>(M_PI);
            angles.roll = 0;
        }
    }

    float angle_normalize(float angle)
    {
        while (angle < -180.0f) angle += 360.0f;
        while (angle > 180.0f) angle -= 360.0f;
        return angle;
    }

    void vector_angles(const Vector& forward, QAngle& angles)
    {
        if (forward.IsZero())
        {
            angles = QAngle(0,0,0);
            return;
        }

        if (forward[1] == 0.0f && forward[0] == 0.0f)
        {
            angles[0] = (forward[2] > 0.0f) ? 270.0f : 90.0f;
            angles[1] = 0.0f;
        }
        else
        {
            float len2d = forward.Length2D();
            if (len2d < 0.001f)
                len2d = 0.001f;

            angles[0] = atan2f(-forward[2], len2d) * -180.0f / static_cast<float>(M_PI);
            angles[1] = atan2f(forward[1], forward[0]) * 180.0f / static_cast<float>(M_PI);

            // Fixed original buggy logic
            if (angles[1] > 180.0f) angles[1] -= 360.0f;
            if (angles[1] < -180.0f) angles[1] += 360.0f;
        }

        angles[2] = 0.0f;
    }

    float clamp(float value, float min, float max)
    {
        if (min > max)
            std::swap(min, max);
        return std::clamp(value, min, max);
    }

    float random_float(float min, float max);
    void random_seed(int seed);
};

extern c_math* g_math;
