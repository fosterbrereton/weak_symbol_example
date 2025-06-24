# Weak Symbol Linking Demonstration

This project demonstrates **weak symbol linking** across shared library (DLL) boundaries on macOS, specifically focusing on C++ RTTI (Run-Time Type Information) functionality including `dynamic_cast` and type unification.

## Quick Start

```bash
# Clone or download this project
# Run the automated build and demonstration:
./build.sh
```

The script will build the project and run a comprehensive test suite using Google Test, demonstrating weak symbol linking in action.

## Overview

The example shows how the same C++ class can be defined in both a host application and a shared library, and through weak symbol linking, the runtime can reconcile these definitions as the same type. This enables:

- ✅ **Cross-boundary RTTI**: `dynamic_cast` works correctly between host and DLL
- ✅ **Type unification**: Objects created in DLL can be cast and used in host with full type safety
- ✅ **Virtual function dispatch**: Virtual methods work correctly across boundaries
- ✅ **Template instantiation sharing**: Template specializations are unified between host and DLL
- ✅ **Exception handling**: Exception types are properly unified (implicit in RTTI)

## Project Structure

```
weak_symbol_example/
├── CMakeLists.txt              # Build configuration with weak symbol support
├── build.sh                    # Automated build and test script
├── .gitignore                  # Git ignore patterns
├── include/
│   ├── base_types.h           # Base classes and interfaces
│   └── shared_class.h         # SharedWorker class with inline definitions
├── lib/
│   ├── shared_library.h       # DLL interface and exports
│   └── shared_library.cpp     # DLL implementation with weak symbols
└── src/
    ├── main.cpp               # Google Test-based demonstration suite
    └── host_implementation.cpp # Host-side weak symbol definitions and tests
```

## Key Components

### 1. Base Types (`include/base_types.h`)
- `IBaseObject`: Abstract interface with virtual methods
- `AbstractWorker`: Intermediate base class
- Proper symbol visibility macros for macOS

### 2. Shared Class (`include/shared_class.h`)
- `SharedWorker`: The class defined in both host and DLL
- `TemplatedWorker<T>`: Template class with explicit instantiations
- Inline definitions that create weak symbols when included in multiple compilation units

### 3. DLL Interface (`lib/shared_library.h` & `.cpp`)
- Factory functions to create instances within the DLL
- RTTI testing utilities
- C-style interface for additional testing
- Weak function implementations using `__attribute__((weak))`

### 4. Host Implementation (`src/host_implementation.cpp`)
- Mirror implementations of DLL weak symbols
- Host-side factory functions
- Cross-boundary type verification functions
- Comprehensive Google Test suite for weak symbol functionality

### 5. Main Test Suite (`src/main.cpp`)
- Google Test-based comprehensive test suite
- RTTI tests across boundaries
- Type unification verification
- Template instantiation tests
- C interface testing

## Technical Implementation

### Weak Symbol Strategy

On macOS, we use `__attribute__((weak))` to create weak symbol definitions:

```cpp
// In both host and DLL
std::string __attribute__((weak)) getSharedFunctionResult() {
    return "Implementation";
}

// Template specializations with weak linkage
template class __attribute__((weak)) TemplatedWorker<int>;
```

### Symbol Visibility

```cpp
#ifdef BUILDING_DLL
    #define API_EXPORT __attribute__((visibility("default")))
#else
    #define API_EXPORT
#endif
```

### RTTI Requirements

- RTTI enabled with `-frtti`
- Consistent vtable layout
- Unified type_info objects
- Proper exception specifications

## Building the Project

### Prerequisites

- macOS with Xcode command line tools
- CMake 3.16 or later
- C++14 compatible compiler (Clang recommended)
- Internet connection (for Google Test download)

### Build Steps

#### Using the Build Script (Recommended)
```bash
# Run the automated build and test script
./build.sh
```

#### Manual Build Steps
```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
make

# Run the test suite
./WeakSymbolHost
```

### Build Configuration

The CMakeLists.txt includes important flags for weak symbol linking and Google Test integration:

```cmake
# C++14 standard and RTTI support
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -Werror")

# Google Test integration
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Platform-specific settings for macOS
if(APPLE)
    # Critical flags for weak symbol linking:
    # -fno-common: Prevents common symbols, ensures proper symbol resolution
    # -fvisibility=default: Makes symbols visible for RTTI unification
    target_compile_options(WeakSymbolLib PRIVATE -fno-common -fvisibility=default)
    target_compile_options(WeakSymbolHost PRIVATE -fno-common -fvisibility=default)
    
    # Shared library linker configuration:
    # -Wl,-flat_namespace: Flattens symbol namespace for symbol interposition
    # -Wl,-undefined,suppress: Allows undefined symbols (resolved at runtime)
    set_target_properties(WeakSymbolLib PROPERTIES
        LINK_FLAGS "-Wl,-flat_namespace -Wl,-undefined,suppress"
        CXX_VISIBILITY_PRESET default
        VISIBILITY_INLINES_HIDDEN OFF
    )
    
    # Host application linker configuration:
    # -Wl,-force_load: Forces loading of ALL symbols for unification
    set_target_properties(WeakSymbolHost PROPERTIES
        LINK_FLAGS "-Wl,-flat_namespace -Wl,-undefined,suppress -Wl,-force_load,${CMAKE_CURRENT_BINARY_DIR}/libWeakSymbolLib.dylib"
    )
endif()
```

## Expected Output

When run successfully, the application will execute a comprehensive Google Test suite demonstrating:

1. **Basic Functionality**: Objects created in host vs DLL work identically
2. **RTTI Tests**: `dynamic_cast` succeeds across boundaries
3. **Type Unification**: `typeid()` consistency for host/DLL objects
4. **Template Unification**: Template specializations are unified
5. **C Interface**: C-style interface also works with RTTI
6. **Weak Symbol Functions**: Functions defined in both locations resolve correctly

Example output:
```
[==========] Running 8 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 8 tests from WeakSymbolLinking
[ RUN      ] WeakSymbolLinking.BasicFunctionality
[ PASSED   ] WeakSymbolLinking.BasicFunctionality (X ms)
[ RUN      ] WeakSymbolLinking.RTTIFunctionality
[ PASSED   ] WeakSymbolLinking.RTTIFunctionality (X ms)
[ RUN      ] WeakSymbolLinking.TypeUnification
[ PASSED   ] WeakSymbolLinking.TypeUnification (X ms)
[ RUN      ] WeakSymbolLinking.TemplateUnification
[ PASSED   ] WeakSymbolLinking.TemplateUnification (X ms)
[ RUN      ] WeakSymbolLinking.CInterface
[ PASSED   ] WeakSymbolLinking.CInterface (X ms)
[ RUN      ] WeakSymbolLinking.WeakSymbolFunctions
[ PASSED   ] WeakSymbolLinking.WeakSymbolFunctions (X ms)
[----------] 8 tests from WeakSymbolLinking (X ms total)
[==========] 8 tests from 1 test suite ran. (X ms total)
[  PASSED  ] 8 tests.

📊 Symbol Analysis:
DLL symbols (SharedWorker related):
[Symbol analysis output using nm and c++filt]
```

## Key Demonstration Points

### 1. Same Type Across Boundaries
Objects created in the DLL can be successfully cast to their concrete types in the host application.

### 2. Unified vtables
Virtual function calls work correctly regardless of where the object was created.

### 3. Template Unification
Template instantiations like `TemplatedWorker<int>` are unified between host and DLL.

### 4. Weak Symbol Resolution
Functions defined with weak symbols in both host and DLL resolve to a single implementation at runtime.

## Troubleshooting

### Common Issues

1. **Linking Errors**: Ensure `-Wl,-flat_namespace -Wl,-undefined,suppress` flags are set
2. **Symbol Visibility**: Make sure API_EXPORT is properly defined
3. **RTTI Disabled**: Verify `-frtti` flag is set
4. **Wrong Architecture**: Ensure consistent architecture (x86_64 or arm64)
5. **Google Test Download**: Ensure internet connection for automatic Google Test download

### Debugging

```bash
# Check symbols in the shared library
nm libWeakSymbolLib.dylib | c++filt | grep SharedWorker

# Check for weak symbols in the host
nm WeakSymbolHost | c++filt | grep -i weak

# Verify RTTI is enabled
otool -L ./WeakSymbolHost

# Run the build script for comprehensive analysis
./build.sh
```

## Platform Notes

This example is specifically designed for **macOS** with the following considerations:

- Uses macOS-specific weak symbol syntax (`__attribute__((weak))`)
- Configured for macOS dynamic linking behavior with flat namespace
- Includes Google Test integration for comprehensive testing
- May require adaptation for Linux (similar) or Windows (different approach needed)

## Learning Outcomes

After running this example, you will understand:

1. How weak symbols enable type unification across shared library boundaries
2. The requirements for RTTI to work across DLL boundaries
3. How to properly configure symbol visibility in shared libraries
4. The relationship between vtables, type_info, and dynamic_cast
5. Practical techniques for building robust cross-boundary C++ APIs
6. How to test complex linking scenarios using Google Test

## Demonstration Results

### What We Successfully Demonstrated ✅

After running the test suite, we confirmed that weak symbol linking on macOS enables:

1. **Cross-Boundary RTTI**: `dynamic_cast` operations work correctly between host and DLL
2. **Virtual Function Dispatch**: Methods called on objects created in DLL work properly in host
3. **Type Compatibility**: Objects can be cast and used polymorphically across boundaries
4. **Template Instantiation**: Templated classes work correctly across boundaries
5. **C Interface Integration**: Mixed C/C++ interfaces work with full RTTI support

### Key Findings 📊

From symbol analysis (`nm` output), we discovered:

- **Multiple Definitions Coexist**: Each binary (host/DLL) contains its own copy of class methods
- **Functional Unification**: `dynamic_cast` and virtual dispatch work correctly across boundaries
- **Weak Functions**: Functions marked with `__attribute__((weak))` exist in both binaries as intended
- **Template Unification**: Template specializations work consistently across boundaries

### Test Results Analysis

The Google Test suite verifies:

```
Cross-boundary casting test:
Cast DLL object to SharedWorker: SUCCESS ✓
Cast HOST object to SharedWorker: SUCCESS ✓
Template casting tests: SUCCESS ✓
C interface tests: SUCCESS ✓
Weak symbol function tests: SUCCESS ✓
```

### macOS-Specific Behavior

On macOS with Clang, weak symbols provide:

- **Compatible vtables**: Virtual dispatch works across boundaries
- **Consistent type names**: RTTI name mangling is identical
- **Successful dynamic_cast**: Type hierarchy is properly recognized
- **Safe polymorphism**: No crashes or undefined behavior

### Practical Implications

This technique enables:

- **Plugin Architectures**: DLLs can return objects that work seamlessly in host
- **Modular Design**: Shared interfaces with distributed implementations
- **Legacy Integration**: Mixing C and C++ code with full type safety
- **Cross-Library Polymorphism**: Objects from different libraries work together

## Further Exploration

Consider experimenting with:

- Different compiler optimizations (`-O2`, `-O3`)
- Different template types and specializations
- Exception handling across boundaries
- Multiple inheritance scenarios
- Plugin architectures using this technique
- Comparison with Linux behavior (likely better unification)
- Windows alternatives (COM, module definition files)
- Adding more comprehensive Google Test scenarios 