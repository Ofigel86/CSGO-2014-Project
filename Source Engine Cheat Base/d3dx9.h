// Stub d3dx9.h for building without legacy DirectX SDK
// Original code doesn't actually use D3DX functions, only includes header
// This stub allows compilation on modern Windows SDK
#pragma once
#include <d3d9.h>
// No D3DX functions needed - ImGui and cheat don't use them
