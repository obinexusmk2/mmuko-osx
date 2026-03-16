#!/bin/bash
# Master build for rift_tompgy - C, C++, C#

echo "Building RIFT Tompgy (Trident Topology Resolver)"

# 1. Build C core library
echo "=== Building C Core Library ==="
cd build && ./build_c.sh
if [ $? -ne 0 ]; then exit 1; fi

# 2. Build C++ wrapper
echo "=== Building C++ Executable ==="
./build_cpp.sh
if [ $? -ne 0 ]; then exit 1; fi

# 3. Build C# application
echo "=== Building C# Application ==="
./build_csharp.sh
if [ $? -ne 0 ]; then exit 1; fi

echo "✅ All components built successfully!"
echo "Run: ./bin/cpp/rift_tompgy --help"
echo "Run: dotnet run --project src/csharp/RiftTompgy.csproj"