@echo off
echo Building CSGO SDK for target version (2014-10-23, VClient016)
echo CInput updated to 0xA4, weapon indices +1
msbuild "Source Engine Cheat Base.sln" /p:Configuration=Release /p:Platform=Win32 /p:PlatformToolset=v143 /verbosity:minimal
if exist "Source Engine Cheat Base\compile\csgosdk.dll" (
    echo Build SUCCESS: Source Engine Cheat Base\compile\csgosdk.dll
    dir "Source Engine Cheat Base\compile\csgosdk.dll"
) else (
    echo Build FAILED - DLL not found
    exit /b 1
)
