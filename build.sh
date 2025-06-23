#!/bin/bash

set -e  # Exit on error

echo "=== Weak Symbol Linking Demonstration Build Script ==="
echo "Platform: $(uname -s)"
echo "Architecture: $(uname -m)"

# Clean previous build
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

# Create build directory
echo "Creating build directory..."
mkdir build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake ..

# Build the project
echo "Building project..."
make

# Check if build was successful
if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Build completed successfully!"
    echo ""
    
    # Show generated files
    echo "Generated files:"
    # adding `|| true` keeps the script from exiting if the files don't exist
    # See https://unix.stackexchange.com/a/325727/8688
    ls -la *.dylib *.exe WeakSymbolHost 2>/dev/null || true
    echo ""

    # Run the demonstration
    echo "🚀 Running demonstration..."
    echo "========================================"
    ./WeakSymbolHost
    
    echo ""
    echo "========================================"
    echo "✅ Demonstration completed!"
    echo ""
    
    # Show symbol analysis
    echo "📊 Symbol Analysis:"
    echo "DLL symbols (SharedWorker related):"
    nm libWeakSymbolLib.dylib | c++filt | grep ' WeakSymbolExample::' | head -5
    echo ""
    echo "Host symbols (SharedWorker related):"
    nm WeakSymbolHost | c++filt | grep ' WeakSymbolExample::' | head -5
    echo ""
    
    echo "🎉 Weak symbol linking demonstration completed successfully!"
else
    echo "❌ Build failed!"
    exit 1
fi 