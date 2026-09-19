#pragma once
#include <cstdint>
#include <Windows.h>
#include <Psapi.h>

#define INRANGE(x, a, b) (x >= a && x <= b)  //-V1003
#define GETBITS(x) (INRANGE((x & (~0x20)),'A','F') ? ((x & (~0x20)) - 'A' + 0xA) : (INRANGE(x, '0', '9') ? x - '0' : 0)) //-V1003
#define GETBYTE(x) (GETBITS(x[0]) << 4 | GETBITS(x[1]))

class c_scanners
{
public:
	uint64_t find_signature(const char* szModule, const char* szSignature);
};

extern c_scanners* g_scanners;