#include "hooking_manager.hpp"

vmthook::vmthook() 
{
    m_ClassBase = nullptr;
    m_OldVT = nullptr;
    m_NewVT = nullptr;
    m_VTSize = 0;
}

vmthook::vmthook(PDWORD* ppdwClassBase) 
{
    m_ClassBase = nullptr;
    m_OldVT = nullptr;
    m_NewVT = nullptr;
    m_VTSize = 0;
    initialize(ppdwClassBase);
}

vmthook::~vmthook() 
{
    unhook();
    if (m_NewVT)
    {
        delete[] m_NewVT;
        m_NewVT = nullptr;
    }
}

bool vmthook::initialize(PDWORD* ppdwClassBase) 
{
    if (!ppdwClassBase || !*ppdwClassBase)
        return false;

    m_ClassBase = ppdwClassBase;
    m_OldVT = *ppdwClassBase;
    
    if (!m_OldVT)
        return false;

    m_VTSize = get_vt_count(m_OldVT);

    if (m_VTSize == 0 || m_VTSize > 1024) // sanity check
        return false;

    m_NewVT = new DWORD[m_VTSize + 1]();
    if (!m_NewVT)
        return false;

    // Safely try to copy RTTI pointer at [-1] if readable
    __try
    {
        if (m_OldVT[-1] != 0 && !IsBadReadPtr(&m_OldVT[-1], sizeof(DWORD)))
            m_NewVT[0] = m_OldVT[-1];
        else
            m_NewVT[0] = 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        m_NewVT[0] = 0;
    }

    memcpy(&m_NewVT[1], m_OldVT, sizeof(DWORD) * m_VTSize);

    DWORD old = 0;
    if (!VirtualProtect(ppdwClassBase, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &old))
        return false;

    *ppdwClassBase = &m_NewVT[1];

    VirtualProtect(ppdwClassBase, sizeof(DWORD), old, &old);

    return true;
}

bool vmthook::initialize(PDWORD** pppdwClassBase) 
{
    if (!pppdwClassBase || !*pppdwClassBase)
        return false;
    return initialize(*pppdwClassBase);
}

void vmthook::clear_class_base() 
{
    m_ClassBase = nullptr;
}

void vmthook::unhook() 
{
    if (m_ClassBase && m_OldVT)
    {
        DWORD old = 0;
        if (VirtualProtect(m_ClassBase, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &old))
        {
            *m_ClassBase = m_OldVT;
            VirtualProtect(m_ClassBase, sizeof(DWORD), old, &old);
        }
    }
}

void vmthook::rehook()
{
    if (m_ClassBase && m_NewVT) 
    {
        DWORD old = 0;
        if (VirtualProtect(m_ClassBase, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &old))
        {
            *m_ClassBase = &m_NewVT[1];
            VirtualProtect(m_ClassBase, sizeof(DWORD), old, &old);
        }
    }
}

int vmthook::get_func_count() {
    return static_cast<int>(m_VTSize);
}

PDWORD vmthook::get_old_vt()
{
    return m_OldVT;
}

DWORD vmthook::hook_function(DWORD dwNewFunc, unsigned int iIndex) 
{
    if (m_NewVT && m_OldVT && iIndex < m_VTSize) 
    {
        m_NewVT[iIndex + 1] = dwNewFunc;
        return m_OldVT[iIndex];
    }

    return 0;
}

DWORD vmthook::get_vt_count(PDWORD pdwVMT) 
{
    if (!pdwVMT)
        return 0;

    DWORD dwIndex = 0;
    // Safer counting with IsBadReadPtr and max limit
    for (dwIndex = 0; dwIndex < 1024; dwIndex++)
    {
        __try
        {
            if (IsBadCodePtr(reinterpret_cast<FARPROC>(pdwVMT[dwIndex])))
                break;
            if (pdwVMT[dwIndex] == 0)
                break;
            // Check if pointer is valid code
            if (IsBadReadPtr(reinterpret_cast<void*>(pdwVMT[dwIndex]), 1))
                break;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            break;
        }
    }

    return dwIndex;
}
