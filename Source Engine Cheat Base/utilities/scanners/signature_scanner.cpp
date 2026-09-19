#include "signature_scanner.hpp"

c_scanners* g_scanners = new c_scanners();

uint64_t c_scanners::find_signature(const char* szModule, const char* szSignature)
{
    if (!szModule || !szSignature)
        return 0;

    HMODULE hMod = GetModuleHandleA(szModule);
    if (!hMod)
        return 0;

    MODULEINFO modInfo{};
    if (!GetModuleInformation(GetCurrentProcess(), hMod, &modInfo, sizeof(MODULEINFO)))
        return 0;

    if (!modInfo.lpBaseOfDll || modInfo.SizeOfImage == 0)
        return 0;

    uintptr_t startAddress = reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
    uintptr_t endAddress = startAddress + modInfo.SizeOfImage;

    const char* pat = szSignature;
    uintptr_t firstMatch = 0;

    for (auto pCur = startAddress; pCur < endAddress; pCur++)
    {
        if (!*pat)
            return firstMatch;

        if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == GETBYTE(pat))
        {
            if (!firstMatch)
                firstMatch = pCur;

            if (!pat[2])
                return firstMatch;

            if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
                pat += 3;
            else
                pat += 2;
        }
        else
        {
            pat = szSignature;
            firstMatch = 0;
        }
    }

    return 0;
}
