# RIFT Pipeline - CMake Build Configuration
# RIFT: RIFT Is a Flexible Translator
# 
# Pipeline Stages:
#   000 - Tokenization
#   001 - Process/Procedure (NSIGII LTE Codec)
#   333 - AST Target Program
#   444 - Platform Target (macOS/Windows/Linux)
#   555 - RIFTBridge (Polyglot C/C++/C#)

cmake_minimum_required(VERSION 3.10)
project(RIFTPipeline 
    VERSION 1.0.0 
    DESCRIPTION "RIFT Pipeline - RIFT Is a Flexible Translator"
    LANGUAGES C CXX
)

# C Standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_NSIGII "Enable NSIGII codec integration" ON)
option(ENABLE_THREADS "Enable thread safety" ON)

# Platform detection
if(APPLE)
    set(PLATFORM_MACOS ON)
    add_definitions(-DRIFT_PLATFORM_MACOS)
elseif(WIN32)
    set(PLATFORM_WINDOWS ON)
    add_definitions(-DRIFT_PLATFORM_WINDOWS)
elseif(UNIX)
    set(PLATFORM_LINUX ON)
    add_definitions(-DRIFT_PLATFORM_LINUX)
endif()

# Compiler flags
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror=implicit-function-declaration")
    set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
elseif(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
    set(CMAKE_C_FLAGS_DEBUG "/Zi /Od /DDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "/O2 /DNDEBUG")
endif()

# Find packages
find_library(MATH_LIBRARY m REQUIRED)
if(ENABLE_THREADS)
    find_package(Threads REQUIRED)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/nsigii-codec
    ${CMAKE_CURRENT_SOURCE_DIR}/rift-555
)

# ============================================================================
# NSIGII Codec Library
# ============================================================================
set(NSIGII_SOURCES
    nsigii-codec/nsigii_codec.c
)

add_library(nsigii_codec STATIC ${NSIGII_SOURCES})
target_link_libraries(nsigii_codec ${MATH_LIBRARY})
if(ENABLE_THREADS)
    target_link_libraries(nsigii_codec Threads::Threads)
endif()

# ============================================================================
# RIFT Pipeline Library
# ============================================================================
set(RIFT_PIPELINE_SOURCES
    rift-000/rift_000_tokenizer.c
    rift-001/rift_001_process.c
    rift-333/rift_333_ast.c
    rift-444/rift_444_target.c
    rift-555/rift_555_bridge.c
)

add_library(rift_pipeline STATIC ${RIFT_PIPELINE_SOURCES})
target_link_libraries(rift_pipeline nsigii_codec)
if(ENABLE_THREADS)
    target_link_libraries(rift_pipeline Threads::Threads)
endif()

# ============================================================================
# RIFT Executable
# ============================================================================
add_executable(rift main.c)
target_link_libraries(rift rift_pipeline nsigii_codec)
if(ENABLE_THREADS)
    target_link_libraries(rift Threads::Threads)
endif()

# ============================================================================
# C++ Wrapper Library (RIFTBridge++)
# ============================================================================
set(RIFTBRIDGE_CPP_SOURCES
    rift-555/rift_555_bridge_cpp.cpp
)

add_library(riftbridge_cpp STATIC ${RIFTBRIDGE_CPP_SOURCES})
target_link_libraries(riftbridge_cpp rift_pipeline)

# ============================================================================
# Test Suite
# ============================================================================
if(BUILD_TESTS)
    enable_testing()
    
    # Test executable
    add_executable(rift_tests tests/test_rift.c)
    target_link_libraries(rift_tests rift_pipeline nsigii_codec)
    if(ENABLE_THREADS)
        target_link_libraries(rift_tests Threads::Threads)
    endif()
    
    # Add tests
    add_test(NAME pipeline_basic COMMAND rift_tests basic)
    add_test(NAME tokenizer_test COMMAND rift_tests tokenizer)
    add_test(NAME nsigii_codec_test COMMAND rift_tests nsigii)
    add_test(NAME riftbridge_test COMMAND rift_tests riftbridge)
endif()

# ============================================================================
# Installation
# ============================================================================
install(TARGETS rift rift_pipeline nsigii_codec riftbridge_cpp
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES
    include/rift_pipeline.h
    nsigii-codec/nsigii_codec.h
    rift-555/rift_555_bridge.h
    DESTINATION include/rift
)

# ============================================================================
# Packaging
# ============================================================================
set(CPACK_PACKAGE_NAME "RIFTPipeline")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "RIFT Pipeline - RIFT Is a Flexible Translator")
set(CPACK_PACKAGE_VENDOR "OBINexus Computing")

include(CPack)

# ============================================================================
# Summary
# ============================================================================
message(STATUS "")
message(STATUS "RIFT Pipeline Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Thread Safety: ${ENABLE_THREADS}")
message(STATUS "  NSIGII Codec: ${ENABLE_NSIGII}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "")
