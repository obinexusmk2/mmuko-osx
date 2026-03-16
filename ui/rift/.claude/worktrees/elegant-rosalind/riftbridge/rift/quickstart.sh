#!/bin/bash
#
# RIFT Tomography Build System
# Stage-based compilation for C, C++, C# polyglot support
#
# Usage: ./rift_build.sh [stage]
#   stage = c | cpp | cs | all | clean | test
#

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
SRC_DIR="${PROJECT_ROOT}/src"
INCLUDE_DIR="${PROJECT_ROOT}/include"

# Compiler configurations
CC="${CC:-gcc}"
CXX="${CXX:-g++}"
CSC="${CSC:-mcs}"

CFLAGS="-std=c11 -O2 -Wall -Wextra -I${INCLUDE_DIR}"
CXXFLAGS="-std=c++17 -O2 -Wall -Wextra -I${INCLUDE_DIR}"
CSFLAGS="-target:library -optimize+"

LDFLAGS="-lm"

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# ============================================================
# STAGE C: Pure C Implementation
# ============================================================

build_stage_c() {
    log_info "Building RIFT Stage C (Pure C11)"
    
    mkdir -p "${BUILD_DIR}/obj/c" "${BUILD_DIR}/lib" "${BUILD_DIR}/bin"
    
    # Core objects
    log_info "[1/4] Compiling eze_trident.c (Power/Leader)"
    ${CC} ${CFLAGS} -c "${SRC_DIR}/core/eze_trident.c" \
        -o "${BUILD_DIR}/obj/c/eze_trident.o"
    
    log_info "[2/4] Compiling obi_semver.c (Heart/Soul)"
    ${CC} ${CFLAGS} -c "${SRC_DIR}/core/obi_semver.c" \
        -o "${BUILD_DIR}/obj/c/obi_semver.o"
    
    log_info "[3/4] Compiling iota_matrix.c (Shared Power)"
    ${CC} ${CFLAGS} -c "${SRC_DIR}/bridge/iota_matrix.c" \
        -o "${BUILD_DIR}/obj/c/iota_matrix.o"
    
    log_info "[4/4] Linking librift.a"
    ar rcs "${BUILD_DIR}/lib/librift.a" \
        "${BUILD_DIR}/obj/c"/*.o
    
    log_info "✓ Stage C complete: ${BUILD_DIR}/lib/librift.a"
}

# ============================================================
# STAGE C++: C++ Bridge
# ============================================================

build_stage_cpp() {
    log_info "Building RIFT Stage C++ (ISO C++17)"
    
    mkdir -p "${BUILD_DIR}/obj/cpp" "${BUILD_DIR}/lib"
    
    log_info "[1/2] Compiling cpp_bridge.cpp"
    ${CXX} ${CXXFLAGS} -c "${SRC_DIR}/bridge/cpp_bridge.cpp" \
        -o "${BUILD_DIR}/obj/cpp/cpp_bridge.o"
    
    log_info "[2/2] Linking librift_cpp.a"
    ar rcs "${BUILD_DIR}/lib/librift_cpp.a" \
        "${BUILD_DIR}/obj/cpp"/*.o \
        "${BUILD_DIR}/obj/c"/*.o
    
    log_info "✓ Stage C++ complete: ${BUILD_DIR}/lib/librift_cpp.a"
}

# ============================================================
# STAGE C#: Mono/.NET Bridge
# ============================================================

build_stage_cs() {
    log_info "Building RIFT Stage C# (Mono)"
    
    if ! command -v ${CSC} &> /dev/null; then
        log_warn "C# compiler (mcs) not found, skipping C# stage"
        return 0
    fi
    
    mkdir -p "${BUILD_DIR}/lib"
    
    log_info "[1/1] Compiling RiftBridge.dll"
    ${CSC} ${CSFLAGS} \
        "${SRC_DIR}/bridge/cs_bridge.cs" \
        -out:"${BUILD_DIR}/lib/RiftBridge.dll"
    
    log_info "✓ Stage C# complete: ${BUILD_DIR}/lib/RiftBridge.dll"
}

# ============================================================
# DEMO BUILD
# ============================================================

build_demo() {
    log_info "Building demo executable"
    
    ${CC} ${CFLAGS} \
        "${SRC_DIR}/demo/rift_demo.c" \
        -L"${BUILD_DIR}/lib" -lrift ${LDFLAGS} \
        -o "${BUILD_DIR}/bin/rift_demo"
    
    log_info "✓ Demo built: ${BUILD_DIR}/bin/rift_demo"
}

# ============================================================
# TESTING
# ============================================================

run_tests() {
    log_info "Running RIFT test suite"
    
    if [ ! -f "${BUILD_DIR}/bin/rift_demo" ]; then
        log_error "Demo not built. Run './rift_build.sh all' first"
        exit 1
    fi
    
    log_info "Executing diamond problem test..."
    "${BUILD_DIR}/bin/rift_demo"
    
    log_info "✓ All tests passed"
}

# ============================================================
# CLEAN
# ============================================================

clean_build() {
    log_info "Cleaning build artifacts"
    rm -rf "${BUILD_DIR}"
    log_info "✓ Clean complete"
}

# ============================================================
# QUICK START GUIDE
# ============================================================

print_usage() {
    cat << EOF

RIFT Tomography Build System
=============================

Usage: $0 [command]

Commands:
  c       Build C library only (Stage C)
  cpp     Build C++ bridge (requires Stage C)
  cs      Build C# bridge (requires Mono)
  all     Build all stages + demo
  demo    Build demo executable
  test    Run test suite
  clean   Remove all build artifacts
  help    Show this message

Examples:
  $0 all          # Full build
  $0 c && $0 demo # Quick C-only build
  $0 test         # Run tests

For detailed documentation, see:
  https://github.com/obinexus/rift
  https://github.com/obinexus/riftbridge

EOF
}

# ============================================================
# MAIN ENTRY POINT
# ============================================================

main() {
    case "${1:-all}" in
        c)
            build_stage_c
            ;;
        cpp)
            build_stage_c
            build_stage_cpp
            ;;
        cs)
            build_stage_cs
            ;;
        all)
            build_stage_c
            build_stage_cpp
            build_stage_cs
            build_demo
            log_info "=== Build Complete ==="
            log_info "Library: ${BUILD_DIR}/lib/librift.a"
            log_info "Demo:    ${BUILD_DIR}/bin/rift_demo"
            ;;
        demo)
            build_demo
            ;;
        test)
            run_tests
            ;;
        clean)
            clean_build
            ;;
        help|--help|-h)
            print_usage
            exit 0
            ;;
        *)
            log_error "Unknown command: $1"
            print_usage
            exit 1
            ;;
    esac
}

main "$@"