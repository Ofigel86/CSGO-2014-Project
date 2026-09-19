#pragma once
#define NOMINMAX

#include <Windows.h>
#include <cstdint>
#include <stdexcept>
#include <cassert>

class vmthook
{
public:
    vmthook();
    vmthook(PDWORD* ppdwClassBase);
    ~vmthook();

    bool initialize(PDWORD* ppdwClassBase);
    bool initialize(PDWORD** pppdwClassBase);

    void clear_class_base();
    void unhook();
    void rehook();

    int get_func_count();

    template <typename Fn>
    Fn get_func_address(int Index) 
    {
        if (Index >= 0 && Index < static_cast<int>(m_VTSize) && m_OldVT != nullptr) 
            return reinterpret_cast<Fn>(m_OldVT[Index]);

        return nullptr;
    }

    DWORD get_func_addr(int Index)
    {
        if (Index >= 0 && Index < static_cast<int>(m_VTSize) && m_OldVT != nullptr)
            return m_OldVT[Index];

        return 0;
    }

    PDWORD get_old_vt();
    DWORD hook_function(DWORD dwNewFunc, unsigned int iIndex);

private:
    DWORD get_vt_count(PDWORD pdwVMT);

    PDWORD* m_ClassBase = nullptr;
    PDWORD  m_NewVT = nullptr;
    PDWORD  m_OldVT = nullptr;
    DWORD   m_VTSize = 0;
};

class ProtectGuard
{
public:
    ProtectGuard(void *base, uint32_t len, uint32_t protect)
    {
        this->base = base;
        this->len = len;
        VirtualProtect(base, len, protect, reinterpret_cast<PDWORD>(&old_protect));
    }

    ~ProtectGuard()
    {
        VirtualProtect(base, len, old_protect, reinterpret_cast<PDWORD>(&old_protect));
    }

private:
    void *base = nullptr;
    uint32_t len = 0;
    uint32_t old_protect = 0;
};

class ShadowVTManager 
{
public:
    ShadowVTManager() : class_base(nullptr), method_count(0), shadow_vtable(nullptr), original_vtable(nullptr) {}
    ShadowVTManager(void *base) : class_base(base), method_count(0), shadow_vtable(nullptr), original_vtable(nullptr) {}
    ~ShadowVTManager()
    {
        RestoreTable();
        if (shadow_vtable)
        {
            delete[] shadow_vtable;
            shadow_vtable = nullptr;
        }
    }

    inline void Setup(void *base = nullptr)
    {
        if (base != nullptr)
            class_base = base;

        if (class_base == nullptr)
            return;

        __try {
            original_vtable = *reinterpret_cast<uintptr_t**>(class_base);
            method_count = GetMethodCount(original_vtable);

            if (method_count == 0 || method_count > 1024)
                return;

            shadow_vtable = new uintptr_t[method_count + 1]();

            __try {
                if (original_vtable[-1] && !IsBadReadPtr(&original_vtable[-1], sizeof(uintptr_t)))
                    shadow_vtable[0] = original_vtable[-1];
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                shadow_vtable[0] = 0;
            }

            std::memcpy(&shadow_vtable[1], original_vtable, method_count * sizeof(uintptr_t));

            auto guard = ProtectGuard{ class_base, sizeof(uintptr_t), PAGE_READWRITE };
            *reinterpret_cast<uintptr_t**>(class_base) = &shadow_vtable[1];
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            if (shadow_vtable)
            {
                delete[] shadow_vtable;
                shadow_vtable = nullptr;
            }
        }
    }

    template<typename T>
    inline void Hook(uint32_t index, T method)
    {
        if (!shadow_vtable || index >= method_count)
            return;
        shadow_vtable[index + 1] = reinterpret_cast<uintptr_t>(method);
    }

    inline void Unhook(uint32_t index)
    {
        if (!shadow_vtable || !original_vtable || index >= method_count)
            return;
        shadow_vtable[index + 1] = original_vtable[index];
    }

    template<typename T>
    inline T GetOriginal(uint32_t index)
    {
        if (!original_vtable || index >= method_count)
            return nullptr;
        return reinterpret_cast<T>(original_vtable[index]);
    }

    inline void RestoreTable()
    {
        __try
        {
            if (original_vtable != nullptr && class_base != nullptr)
            {
                auto guard = ProtectGuard{ class_base, sizeof(uintptr_t), PAGE_READWRITE };
                *reinterpret_cast<uintptr_t**>(class_base) = original_vtable;
                original_vtable = nullptr;
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

private:
    inline uint32_t GetMethodCount(uintptr_t *vtable_start)
    {
        if (!vtable_start)
            return 0;

        uint32_t len = 0;
        __try {
            while (len < 1024 && vtable_start[len] && !IsBadCodePtr(reinterpret_cast<FARPROC>(vtable_start[len])))
                ++len;
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
        return len;
    }

    void *class_base;
    uint32_t method_count;
    uintptr_t *shadow_vtable;
    uintptr_t *original_vtable;
};
