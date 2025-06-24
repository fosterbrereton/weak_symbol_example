cmake_minimum_required(VERSION 3.16)
project(WeakSymbolExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# Base compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -Werror")

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

target_compile_definitions(WeakSymbolLib PRIVATE BUILDING_DLL)

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

# Platform-specific settings for macOS
if(APPLE)
    # Configure compilation flags for proper RTTI symbol unification
    target_compile_options(WeakSymbolLib PRIVATE
        -fno-common
        -fvisibility=default
    )
    
    target_compile_options(WeakSymbolHost PRIVATE
        -fno-common
        -fvisibility=default
    )
    
    # Configure shared library for symbol export and interposition
    set_target_properties(WeakSymbolLib PROPERTIES
        LINK_FLAGS "-Wl,-flat_namespace -Wl,-undefined,suppress"
        CXX_VISIBILITY_PRESET default
        VISIBILITY_INLINES_HIDDEN OFF
    )
    
    # Configure host application for symbol unification
    set_target_properties(WeakSymbolHost PROPERTIES
        LINK_FLAGS "-Wl,-flat_namespace -Wl,-undefined,suppress -Wl,-force_load,${CMAKE_CURRENT_BINARY_DIR}/libWeakSymbolLib.dylib"
    )
endif() 