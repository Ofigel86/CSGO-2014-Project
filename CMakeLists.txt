cmake_minimum_required(VERSION 3.15)
project(csgosdk)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This is Windows-only project, but CMake helps with static analysis on Linux
# On Windows, you need DirectX SDK and MSVC v143

if(WIN32)
    file(GLOB_RECURSE SOURCES 
        "Source Engine Cheat Base/*.cpp"
        "Source Engine Cheat Base/*.hpp"
        "Source Engine Cheat Base/*.h"
    )

    # Exclude ImGui demo if you want smaller build
    # list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/Source Engine Cheat Base/gui/drawing/imgui_demo.cpp")

    add_library(csgosdk SHARED ${SOURCES})

    target_include_directories(csgosdk PRIVATE
        "Source Engine Cheat Base"
        $ENV{DXSDK_DIR}/Include
    )

    target_link_directories(csgosdk PRIVATE
        $ENV{DXSDK_DIR}/Lib/x86
    )

    target_link_libraries(csgosdk PRIVATE
        d3d9
        d3dx9
    )

    target_compile_definitions(csgosdk PRIVATE
        WIN32
        NDEBUG
        _CONSOLE
        NOMINMAX
    )

    set_target_properties(csgosdk PROPERTIES
        OUTPUT_NAME "csgosdk"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/compile"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/compile"
    )
else()
    message(STATUS "Non-Windows build: only for syntax checking, not for actual DLL")
    # On Linux, we can at least check that headers are somewhat valid
    # This will fail due to Windows.h, but useful for IDEs
endif()
