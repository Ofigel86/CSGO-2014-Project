#!/bin/bash
set -e
ROOT="Source Engine Cheat Base"
mkdir -p compile_out
# Create temporary stubs for Linux case-sensitive build in src symlink or directly
# Ensure src symlink exists
if [ ! -L src ]; then
  rm -rf src
  ln -s "$ROOT" src
fi
# Create stubs
cat > src/Windows.h << 'STUB'
#pragma once
#include <windows.h>
STUB
cat > src/Psapi.h << 'STUB'
#pragma once
#include <psapi.h>
STUB
cat > src/DirectXMath.h << 'STUB'
#pragma once
#include <cmath>
namespace DirectX { inline void XMScalarSinCos(float* s, float* c, float v) { if(s) *s = sinf(v); if(c) *c = cosf(v); } }
STUB
cat > src/seh_fix.hpp << 'STUB'
#pragma once
#include <cfloat>
#include <cstdint>
#ifdef __clang__
#ifdef __try
#undef __try
#endif
#ifdef __except
#undef __except
#endif
#define __try try
#define __except(x) catch(...)
#ifndef EXCEPTION_EXECUTE_HANDLER
#define EXCEPTION_EXECUTE_HANDLER 1
#endif
#endif
#ifndef byte
typedef unsigned char byte;
#endif
#ifdef __clang__
#ifndef PDIRECT3DDEVICE9
typedef struct IDirect3DDevice9* PDIRECT3DDEVICE9;
typedef struct IDirect3DDevice9* LPDIRECT3DDEVICE9;
#endif
#endif
#include <cfloat>
#include <cmath>
STUB

mkdir -p compile_out
FILES=$(find -L src -name "*.cpp" | tr '\n' ' ')
echo "Building $FILES count $(echo $FILES | wc -w)"
python3 -m ziglang c++ -target x86-windows-gnu -shared -o compile_out/csgosdk.dll $FILES -Isrc -I. -std=c++17 -fms-extensions -include src/seh_fix.hpp -DWIN32 -DNDEBUG -D_CONSOLE -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -O2 -ld3d9 -lpsapi -luser32 -lkernel32 -lgdi32 -lole32 -loleaut32 -limm32 -ldwmapi
cp compile_out/csgosdk.dll "$ROOT/compile/csgosdk.dll"
mkdir -p compile
cp compile_out/csgosdk.dll compile/csgosdk.dll
echo "Built: $(ls -lh compile_out/csgosdk.dll)"
# Cleanup stubs
rm src/Windows.h src/Psapi.h src/DirectXMath.h src/seh_fix.hpp
echo "Cleaned stubs, DLL remains at $ROOT/compile/csgosdk.dll"
