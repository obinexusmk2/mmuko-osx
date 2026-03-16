# ============================================================================
# Makefile - MMUKO-OS Ring Boot Build System
# Project: OBINexus / MMUKO-OS
# Protocol: NSIGII Human Rights Verification Firmware
#
# Build targets:
#   all          - Build C ring boot system (default)
#   ringboot     - Build ring boot executable (C)
#   riftbridge   - Build riftbridge C++ UI library
#   bootimg      - Assemble boot.asm into bootable image
#   csharp       - Build C# riftbridge UI (requires dotnet)
#   clean        - Remove build artifacts
#   run          - Build and run ring boot
#   test         - Run basic tests
#
# Directory layout:
#   asm/         - Assembly boot sector
#   src/         - C/C++ source files
#   include/     - Header files (C and C++)
#   ui/          - C# managed UI layer
#   build/       - Build output directory
#
# Toolchain:
#   CC           - C compiler (gcc)
#   CXX          - C++ compiler (g++)
#   ASM          - Assembler (nasm)
#   DOTNET       - .NET CLI (dotnet)
#   nlink        - OBINexus link orchestrator (optional)
#   polybuild    - OBINexus polyglot build (optional)
# ============================================================================

# Compilers and tools
CC       = gcc
CXX      = g++
ASM      = nasm
DOTNET   = dotnet
AR       = ar

# Directories
SRC_DIR     = src
INC_DIR     = include
ASM_DIR     = asm
UI_DIR      = ui
BUILD_DIR   = build

# Compiler flags
CFLAGS      = -Wall -Wextra -std=c11 -O2 -I$(INC_DIR)
CXXFLAGS    = -Wall -Wextra -std=c++17 -O2 -I$(INC_DIR)
LDFLAGS     = -lm
ASMFLAGS    = -f bin

# Debug flags
DEBUG_CFLAGS   = -Wall -Wextra -std=c11 -g -O0 -I$(INC_DIR) -DDEBUG
DEBUG_CXXFLAGS = -Wall -Wextra -std=c++17 -g -O0 -I$(INC_DIR) -DDEBUG

# Source files
C_SOURCES   = $(SRC_DIR)/bootsec.c $(SRC_DIR)/ringboot.c
CXX_SOURCES = $(SRC_DIR)/riftbridge.cpp
C_UI_SRC    = $(SRC_DIR)/riftbridge.c
ASM_SOURCE  = $(ASM_DIR)/boot.asm
CS_SOURCE   = $(UI_DIR)/riftbridge.cs

# Object files
C_OBJECTS   = $(BUILD_DIR)/bootsec.o $(BUILD_DIR)/ringboot.o
CXX_OBJECTS = $(BUILD_DIR)/riftbridge_cpp.o
C_UI_OBJ    = $(BUILD_DIR)/riftbridge_c.o

# Targets
RINGBOOT    = $(BUILD_DIR)/ringboot
BOOTIMG     = $(BUILD_DIR)/mmuko-os.img
LIBRIFT_A   = $(BUILD_DIR)/libriftbridge.a
LIBRIFT_SO  = $(BUILD_DIR)/libriftbridge.so

# ============================================================================
# DEFAULT TARGET
# ============================================================================

.PHONY: all clean run debug bootimg riftbridge csharp test install help

all: dirs ringboot

help:
	@echo "MMUKO-OS Build System"
	@echo "====================="
	@echo "  make all         - Build ring boot (default)"
	@echo "  make ringboot    - Build ring boot executable"
	@echo "  make bootimg     - Assemble boot sector image"
	@echo "  make riftbridge  - Build riftbridge C++ library"
	@echo "  make csharp      - Build C# UI (requires dotnet)"
	@echo "  make run         - Build and run ring boot"
	@echo "  make debug       - Build with debug symbols"
	@echo "  make test        - Run basic tests"
	@echo "  make clean       - Remove build artifacts"

# ============================================================================
# DIRECTORY SETUP
# ============================================================================

dirs:
ifeq ($(OS),Windows_NT)
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
else
	@mkdir -p $(BUILD_DIR)
endif

# ============================================================================
# RING BOOT (C executable)
# ============================================================================

ringboot: dirs $(RINGBOOT)

$(RINGBOOT): $(C_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "[BUILD] Ring boot executable: $@"

$(BUILD_DIR)/bootsec.o: $(SRC_DIR)/bootsec.c $(INC_DIR)/bootsec.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/ringboot.o: $(SRC_DIR)/ringboot.c $(INC_DIR)/ringboot.h $(INC_DIR)/bootsec.h
	$(CC) $(CFLAGS) -c -o $@ $<

# ============================================================================
# BOOT SECTOR IMAGE (NASM assembly)
# ============================================================================

bootimg: dirs $(BOOTIMG)

$(BOOTIMG): $(ASM_SOURCE)
	$(ASM) $(ASMFLAGS) -o $@ $<
	@echo "[ASM] Boot image: $@ ($(shell stat -c%s $@ 2>/dev/null || echo '512') bytes)"

# ============================================================================
# RIFT BRIDGE LIBRARY (C + C++)
# ============================================================================

riftbridge: dirs $(LIBRIFT_A) $(LIBRIFT_SO)

$(LIBRIFT_A): $(C_UI_OBJ) $(CXX_OBJECTS) $(BUILD_DIR)/bootsec.o
	$(AR) rcs $@ $^
	@echo "[BUILD] Static library: $@"

$(LIBRIFT_SO): $(C_UI_OBJ) $(CXX_OBJECTS) $(BUILD_DIR)/bootsec.o
	$(CXX) -shared -o $@ $^ $(LDFLAGS)
	@echo "[BUILD] Shared library: $@"

$(BUILD_DIR)/riftbridge_c.o: $(SRC_DIR)/riftbridge.c $(INC_DIR)/riftbridge.h
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

$(BUILD_DIR)/riftbridge_cpp.o: $(SRC_DIR)/riftbridge.cpp $(INC_DIR)/riftbridge.hpp $(INC_DIR)/riftbridge.h
	$(CXX) $(CXXFLAGS) -fPIC -c -o $@ $<

# ============================================================================
# C# UI (requires .NET SDK)
# ============================================================================

csharp:
	@if command -v $(DOTNET) > /dev/null 2>&1; then \
		echo "[DOTNET] Building C# riftbridge UI..."; \
		mkdir -p $(BUILD_DIR)/csharp; \
		$(DOTNET) build $(UI_DIR)/riftbridge.cs -o $(BUILD_DIR)/csharp 2>/dev/null || \
		echo "[DOTNET] Note: Requires .NET project setup. See ui/ directory."; \
	else \
		echo "[SKIP] dotnet not found - skipping C# build"; \
	fi

# ============================================================================
# RUN & TEST
# ============================================================================

run: ringboot
	@echo ""
	@echo "=== Running MMUKO-OS Ring Boot ==="
	@echo ""
	@./$(RINGBOOT)

test: ringboot
	@echo ""
	@echo "=== MMUKO-OS Ring Boot Tests ==="
	@echo ""
	@./$(RINGBOOT) && echo "[TEST] Ring boot: PASS" || echo "[TEST] Ring boot: FAIL"
	@if [ -f $(BOOTIMG) ]; then \
		SIZE=$$(stat -c%s $(BOOTIMG) 2>/dev/null || echo 0); \
		if [ "$$SIZE" = "512" ]; then \
			echo "[TEST] Boot image size (512 bytes): PASS"; \
		else \
			echo "[TEST] Boot image size ($$SIZE bytes): FAIL (expected 512)"; \
		fi; \
		LAST2=$$(xxd -s 510 -l 2 -p $(BOOTIMG) 2>/dev/null || echo "0000"); \
		if [ "$$LAST2" = "55aa" ]; then \
			echo "[TEST] Boot signature (0xAA55): PASS"; \
		else \
			echo "[TEST] Boot signature ($$LAST2): FAIL (expected 55aa)"; \
		fi; \
	fi

# ============================================================================
# DEBUG BUILD
# ============================================================================

debug: CFLAGS = $(DEBUG_CFLAGS)
debug: CXXFLAGS = $(DEBUG_CXXFLAGS)
debug: dirs ringboot
	@echo "[BUILD] Debug build complete"

# ============================================================================
# STATIC ANALYSIS
# ============================================================================

analyze:
	@echo "[ANALYZE] Running cppcheck on sources..."
	@cppcheck --enable=all --inconclusive -I$(INC_DIR) $(SRC_DIR)/*.c 2>&1 || true

format:
	@echo "[FORMAT] Formatting sources..."
	@clang-format -i $(SRC_DIR)/*.c $(SRC_DIR)/*.cpp $(INC_DIR)/*.h $(INC_DIR)/*.hpp 2>/dev/null || true

# ============================================================================
# CLEAN
# ============================================================================

clean:
ifeq ($(OS),Windows_NT)
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
else
	rm -rf $(BUILD_DIR)
endif
	@echo "[CLEAN] Build artifacts removed"

# ============================================================================
# INSTALL (to system or OBINexus toolchain)
# ============================================================================

install: all bootimg riftbridge
	@echo "[INSTALL] Build artifacts:"
	@echo "  $(RINGBOOT)"
	@echo "  $(BOOTIMG)"
	@echo "  $(LIBRIFT_A)"
	@echo "  $(LIBRIFT_SO)"
	@echo ""
	@echo "Integration with OBINexus toolchain:"
	@echo "  nlink → polybuild → mmuko-os"
	@echo "  riftlang.exe → .so.a → rift.exe → gosilang"
