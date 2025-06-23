cmake_minimum_required(VERSION 3.16)
project(WeakSymbolExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable RTTI and proper symbol visibility
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -fvisibility=hidden")

# Create include directory
include_directories(include)

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

# Host Application
add_executable(WeakSymbolHost
    src/main.cpp
    src/host_implementation.cpp
)

# Link the shared library
target_link_libraries(WeakSymbolHost WeakSymbolLib)

# Enable debugging symbols
set(CMAKE_BUILD_TYPE Debug)

# Platform-specific settings for macOS
if(APPLE)
    # Ensure proper symbol resolution
    target_link_options(WeakSymbolLib PRIVATE -undefined dynamic_lookup)
    target_link_options(WeakSymbolHost PRIVATE -Wl,-bind_at_load)
endif() 