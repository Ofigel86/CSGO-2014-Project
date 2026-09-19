#pragma once
#include <cstdint>

template <typename FuncType>
__forceinline static FuncType call_virtual(void* ppClass, int index)
{
    if (!ppClass)
        return nullptr;
    uintptr_t* pVTable = *reinterpret_cast<uintptr_t**>(ppClass);
    if (!pVTable)
        return nullptr;
    uintptr_t dwAddress = pVTable[index];
    return reinterpret_cast<FuncType>(dwAddress);
}
