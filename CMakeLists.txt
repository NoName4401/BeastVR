cmake_minimum_required(VERSION 3.24)
project(BeastVR VERSION 0.0.1 LANGUAGES C CXX)

if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "BeastVR builds for Windows x64 only.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Static MSVC runtime: beastvr.dll is loaded into the game's process, where
# Windows will NOT look in our folder for extra DLLs. Everything we use must be
# linked in statically.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---------------------------------------------------------------- dependencies
include(FetchContent)

FetchContent_Declare(minhook
  GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
  GIT_TAG        8af6b4acae5a9388fd742b56fa79ece89d96f823)

FetchContent_Declare(spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.17.0
  GIT_SHALLOW    TRUE)

FetchContent_Declare(openxr
  GIT_REPOSITORY https://github.com/KhronosGroup/OpenXR-SDK.git
  GIT_TAG        release-1.1.65
  GIT_SHALLOW    TRUE)

set(BUILD_TESTS            OFF CACHE BOOL "" FORCE)
set(BUILD_API_LAYERS       OFF CACHE BOOL "" FORCE)
set(DYNAMIC_LOADER         OFF CACHE BOOL "" FORCE)  # static OpenXR loader, see note above
set(SPDLOG_BUILD_EXAMPLE   OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(minhook spdlog openxr)

# The OpenXR SDK forces the DLL runtime (/MD) on its static loader. Override it
# to match ours, or linking fails with a runtime library mismatch.
set_target_properties(openxr_loader PROPERTIES
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(BEASTVR_DEFS WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE
    BEASTVR_VERSION="${PROJECT_VERSION}")

# ---------------------------------------------------------------- beastvr.dll
add_library(beastvr_core SHARED
  src/core/dllmain.cpp
  src/core/log.cpp
  src/core/hooks/present_hook.cpp
  src/core/xr/xr_probe.cpp)

set_target_properties(beastvr_core PROPERTIES OUTPUT_NAME "beastvr")
target_include_directories(beastvr_core PRIVATE
  src/core
  ${minhook_SOURCE_DIR}/include
  ${openxr_SOURCE_DIR}/include)
target_compile_definitions(beastvr_core PRIVATE ${BEASTVR_DEFS})
target_compile_options(beastvr_core PRIVATE /W4 /utf-8 /permissive-)
target_link_libraries(beastvr_core PRIVATE
  minhook spdlog::spdlog openxr_loader
  d3d11 dxgi shell32 ole32)

# ---------------------------------------------------------------- launcher
add_executable(beastvr_launcher src/launcher/main.cpp)
set_target_properties(beastvr_launcher PROPERTIES OUTPUT_NAME "BeastVR Launcher")
target_compile_definitions(beastvr_launcher PRIVATE ${BEASTVR_DEFS})
target_compile_options(beastvr_launcher PRIVATE /W4 /utf-8 /permissive-)

# ---------------------------------------------------------------- config
# Copy the ini template once. After that it's yours to edit; rebuilding never
# overwrites it.
if(NOT EXISTS ${CMAKE_BINARY_DIR}/bin/beastvr.ini)
  file(COPY ${CMAKE_SOURCE_DIR}/config/beastvr.ini DESTINATION ${CMAKE_BINARY_DIR}/bin)
endif()
