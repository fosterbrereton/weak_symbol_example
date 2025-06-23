cmake_minimum_required(VERSION 3.16)
project(WeakSymbolExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable RTTI and proper symbol visibility, treat warnings as errors
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -fvisibility=hidden -Werror")

# Create include directory
include_directories(include)

# Fetch Google Test using FetchContent
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP true
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Shared Library (DLL equivalent on macOS)
add_library(WeakSymbolLib SHARED
    lib/shared_library.cpp
)

# Set proper visibility and weak symbol support
target_compile_definitions(WeakSymbolLib PRIVATE BUILDING_DLL)
target_compile_options(WeakSymbolLib PRIVATE
    -fvisibility=hidden
    -fvisibility-inlines-hidden
)

# Host Application with Google Test
add_executable(WeakSymbolHost
    src/main.cpp
    src/host_implementation.cpp
)

# Link the shared library and Google Test
target_link_libraries(WeakSymbolHost 
    WeakSymbolLib 
    gtest 
    gtest_main
)

# Enable debugging symbols
set(CMAKE_BUILD_TYPE Debug)

# Platform-specific settings for macOS
if(APPLE)
    # Ensure proper symbol resolution
    target_link_options(WeakSymbolLib PRIVATE -undefined dynamic_lookup)
    target_link_options(WeakSymbolHost PRIVATE -Wl)
endif() 