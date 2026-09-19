#include "math.hpp"

#include <Windows.h>

c_math* g_math = new c_math;

void c_math::random_seed(int iSeed)
{
	typedef void(__cdecl* RandomSeed_t)(int);
	static RandomSeed_t pRandomSeed = (RandomSeed_t)(GetProcAddress(GetModuleHandle("vstdlib"), "RandomSeed"));
	pRandomSeed(iSeed);
}

float c_math::random_float(float fMin, float fMax)
{
	typedef float(__cdecl* RandomFloat_t)(float, float);
	static RandomFloat_t pRandomFloat = (RandomFloat_t)(GetProcAddress(GetModuleHandle("vstdlib"), "RandomFloat"));
	return pRandomFloat(fMin, fMax);
}