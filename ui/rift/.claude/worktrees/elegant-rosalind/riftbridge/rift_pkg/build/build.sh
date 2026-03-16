#!/bin/bash

# RIFT Package Manager Build Script
set -e

echo "Building RIFT Package Manager..."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check for required dependencies
check_dependencies() {
    print_status "Checking dependencies..."
    
    command -v gcc >/dev/null 2>&1 || { print_error "gcc is required but not installed."; exit 1; }
    command -v g++ >/dev/null 2>&1 || { print_warning "g++ not found. C++ support will be limited."; }
    command -v mono >/dev/null 2>&1 || command -v dotnet >/dev/null 2>&1 || { print_warning "C# runtime not found. C# support will be limited."; }
    
    print_status "Dependencies check passed."
}

# Clean previous builds
clean_build() {
    print_status "Cleaning previous builds..."
    make clean
}

# Build all components
build_all() {
    print_status "Building all components..."
    make all
}

# Run tests
run_tests() {
    print_status "Running tests..."
    make run-test
}

# Install system-wide
install_system() {
    print_status "Installing RIFT package manager system-wide..."
    sudo make install
}

# Main build process
main() {
    cd "$(dirname "$0")"
    
    print_status "Starting RIFT Package Manager build process..."
    
    check_dependencies
    clean_build
    build_all
    
    if [ "$1" == "--test" ]; then
        run_tests
    fi
    
    if [ "$1" == "--install" ]; then
        install_system
    fi
    
    print_status "Build completed successfully!"
    print_status "Libraries built in: $(pwd)/lib"
    print_status "Binaries built in: $(pwd)/bin"
}

# Run main function
main "$@"