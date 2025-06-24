cmake_minimum_required(VERSION 3.16)
project(WeakSymbolExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# Base compiler flags
# -frtti: Enable Run-Time Type Information (RTTI) for dynamic_cast and typeid operations
# -Werror: Treat all compiler warnings as errors to ensure clean builds
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -Werror")

# Create include directory
include_directories(include)

# Fetch Google Test using FetchContent
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
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
# These flags are CRITICAL for proper weak symbol linking and RTTI unification
if(APPLE)
    # Compiler flags for both targets:
    # -fno-common: Prevents common symbols, ensuring proper symbol resolution across boundaries
    # -fvisibility=default: Makes all symbols visible by default (overrides hidden visibility)
    #                      This is essential for RTTI type_info unification across library boundaries
    target_compile_options(WeakSymbolLib PRIVATE
        -fno-common
        -fvisibility=default
    )
    
    target_compile_options(WeakSymbolHost PRIVATE
        -fno-common
        -fvisibility=default
    )
    
    # Shared library linker configuration:
    # -Wl,-flat_namespace: Flattens symbol namespace, allowing symbol interposition
    #                      Essential for weak symbol linking - symbols with same name unify
    # -Wl,-undefined,suppress: Allows undefined symbols at link time (resolved at runtime)
    #                          Enables weak linking where symbols may be defined elsewhere
    # CXX_VISIBILITY_PRESET default: CMake setting to export all C++ symbols by default
    # VISIBILITY_INLINES_HIDDEN OFF: Ensures inline functions are also exported for RTTI
    set_target_properties(WeakSymbolLib PROPERTIES
        LINK_FLAGS "-Wl,-flat_namespace -Wl,-undefined,suppress"
        CXX_VISIBILITY_PRESET default
        VISIBILITY_INLINES_HIDDEN OFF
    )
    
    # Host application linker configuration:
    # -Wl,-flat_namespace: Same as above - enables symbol interposition
    # -Wl,-undefined,suppress: Allows weak symbol resolution at runtime
    # -Wl,-force_load: Forces loading of ALL symbols from the shared library
    #                  This ensures weak symbols are available for unification
    #                  Critical for RTTI - ensures type_info symbols are loaded and unified
    set_target_properties(WeakSymbolHost PROPERTIES
        LINK_FLAGS "-Wl,-flat_namespace -Wl,-undefined,suppress -Wl,-force_load,${CMAKE_CURRENT_BINARY_DIR}/libWeakSymbolLib.dylib"
    )
endif()

# IMPORTANT NOTES:
# 1. The -flat_namespace flag is essential for weak symbol behavior on macOS
#    Without it, symbols remain in separate namespaces and don't unify
# 2. The -force_load flag ensures all symbols are loaded, preventing lazy loading
#    that could prevent proper RTTI type_info unification
# 3. Default visibility (-fvisibility=default) is required for RTTI symbols
#    Hidden visibility would prevent type_info unification across boundaries
# 4. These settings work together to create a "Windows DLL-like" behavior on macOS
#    where symbols can be shared and unified across library boundaries 