#pragma once
#include "../../math/Vector.hpp"
#include "../../math/Vector4D.hpp"
#include "../../math/QAngle.hpp"

#define DEG2RAD( x ) ( ( float )( x ) * ( float )( ( float )( M_PI ) / 180.0f ) )
#define RAD2DEG( x ) ( ( float )( x ) * ( float )( 180.0f / ( float )( M_PI ) ) )

class c_math
{
public:
	void sin_cos(float a, float* s, float* c)
	{
		*s = sin(a);
		*c = cos(a);
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
		float	sp, sy, cp, cy;

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
			angles.pitch = atan2f(-forward.z, forwardDist) * 180 / M_PI;
			angles.yaw = atan2f(forward.y, forward.x) * 180 / M_PI;

			float upZ = (left.y * forward.x) - (left.x * forward.y);
			angles.roll = atan2f(left.z, upZ) * 180 / M_PI;
		}
		else
		{
			angles.pitch = atan2f(-forward.z, forwardDist) * 180 / M_PI;
			angles.yaw = atan2f(-left.x, left.y) * 180 / M_PI;
			angles.roll = 0;
		}
	}

	float angle_normalize(float angle)
	{
		while (angle < -180)    angle += 360;
		while (angle > 180)    angle -= 360;

		return angle;
	}

	void vector_angles(const Vector& forward, QAngle& angles)
	{
		if (forward[1] == 0.0f && forward[0] == 0.0f)
		{
			angles[0] = (forward[2] > 0.0f) ? 270.0f : 90.0f;
			angles[1] = 0.0f;
		}
		else
		{
			angles[0] = atan2(-forward[2], forward.Length2D()) * -180 / M_PI;
			angles[1] = atan2(forward[1], forward[0]) * 180 / M_PI;

			if (angles[1] > 90) angles[1] -= 180;
			else if (angles[1] < 90) angles[1] += 180;
			else if (angles[1] == 90) angles[1] = 0;
		}

		angles[2] = 0.0f;
	}

	float clamp(float value, float min, float max)
	{
		float val = value;

		if (value < min)
			val = min;

		if (value > max)
			val = max;

		return val;
	}

	float random_float(float min, float max);
	void random_seed(int seed);
};

extern c_math* g_math;