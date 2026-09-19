#include "math.hpp"

#include <Windows.h>

c_math* g_math = new c_math();

void c_math::random_seed(int iSeed)
{
    if (iSeed < 0)
        iSeed = 0;

    __try {
        typedef void(__cdecl* RandomSeed_t)(int);
        static RandomSeed_t pRandomSeed = nullptr;
        if (!pRandomSeed)
            pRandomSeed = reinterpret_cast<RandomSeed_t>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomSeed"));
        if (pRandomSeed)
            pRandomSeed(iSeed);
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

float c_math::random_float(float fMin, float fMax)
{
    if (fMin > fMax)
        std::swap(fMin, fMax);
    if (fMin == fMax)
        return fMin;

    __try {
        typedef float(__cdecl* RandomFloat_t)(float, float);
        static RandomFloat_t pRandomFloat = nullptr;
        if (!pRandomFloat)
            pRandomFloat = reinterpret_cast<RandomFloat_t>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomFloat"));
        if (pRandomFloat)
            return pRandomFloat(fMin, fMax);
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    // Fallback
    return fMin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (fMax - fMin)));
}
