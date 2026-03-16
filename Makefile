# ============================================================================
# Makefile - MMUKO-OS Ring Boot Build System
# ============================================================================

# Compilers and tools
CC       = gcc
CXX      = g++
ASM      = nasm
DOTNET   = dotnet
AR       = ar

# Directories
SRC_DIR        = src
INC_DIR        = include
ASM_DIR        = asm
UI_DIR         = ui
ARTIFACTS_DIR  = artifacts
C_ARTIFACT_DIR = $(ARTIFACTS_DIR)/c/mmuko-os
CPP_ARTIFACT_DIR = $(ARTIFACTS_DIR)/cpp/mmuko-os
CSHARP_ARTIFACT_DIR = $(ARTIFACTS_DIR)/csharp

# Compiler flags
CFLAGS      = -Wall -Wextra -std=c11 -O2 -I$(INC_DIR)
CXXFLAGS    = -Wall -Wextra -std=c++17 -O2 -I$(INC_DIR)
LDFLAGS     = -lm
ASMFLAGS    = -f bin

# Debug flags
DEBUG_CFLAGS   = -Wall -Wextra -std=c11 -g -O0 -I$(INC_DIR) -DDEBUG
DEBUG_CXXFLAGS = -Wall -Wextra -std=c++17 -g -O0 -I$(INC_DIR) -DDEBUG

# Object files
C_OBJECTS   = $(C_ARTIFACT_DIR)/bootsec.o $(C_ARTIFACT_DIR)/ringboot.o
CXX_OBJECTS = $(CPP_ARTIFACT_DIR)/riftbridge_cpp.o
C_UI_OBJ    = $(CPP_ARTIFACT_DIR)/riftbridge_c.o

# Targets
RINGBOOT    = $(C_ARTIFACT_DIR)/ringboot
BOOTIMG     = $(C_ARTIFACT_DIR)/mmuko-os.img
LIBRIFT_A   = $(CPP_ARTIFACT_DIR)/libriftbridge.a
LIBRIFT_SO  = $(CPP_ARTIFACT_DIR)/libriftbridge.so

.PHONY: all clean run debug bootimg riftbridge csharp test install help dirs

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
	@mkdir -p $(C_ARTIFACT_DIR) $(CPP_ARTIFACT_DIR) $(CSHARP_ARTIFACT_DIR)

# ============================================================================
# RING BOOT (C executable)
# ============================================================================

ringboot: dirs $(RINGBOOT)

$(RINGBOOT): $(C_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "[BUILD] Ring boot executable: $@"

$(C_ARTIFACT_DIR)/bootsec.o: $(SRC_DIR)/bootsec.c $(INC_DIR)/bootsec.h | dirs
	$(CC) $(CFLAGS) -c -o $@ $<

$(C_ARTIFACT_DIR)/ringboot.o: $(SRC_DIR)/ringboot.c $(INC_DIR)/ringboot.h $(INC_DIR)/bootsec.h | dirs
	$(CC) $(CFLAGS) -c -o $@ $<

# ============================================================================
# BOOT SECTOR IMAGE (NASM assembly)
# ============================================================================

bootimg: dirs $(BOOTIMG)

$(BOOTIMG): $(ASM_DIR)/boot.asm | dirs
	$(ASM) $(ASMFLAGS) -o $@ $<
	@echo "[ASM] Boot image: $@ ($(shell stat -c%s $@ 2>/dev/null || echo '512') bytes)"

# ============================================================================
# RIFT BRIDGE LIBRARY (C + C++)
# ============================================================================

riftbridge: dirs $(LIBRIFT_A) $(LIBRIFT_SO)

$(LIBRIFT_A): $(C_UI_OBJ) $(CXX_OBJECTS) $(C_ARTIFACT_DIR)/bootsec.o
	$(AR) rcs $@ $^
	@echo "[BUILD] Static library: $@"

$(LIBRIFT_SO): $(C_UI_OBJ) $(CXX_OBJECTS) $(C_ARTIFACT_DIR)/bootsec.o
	$(CXX) -shared -o $@ $^ $(LDFLAGS)
	@echo "[BUILD] Shared library: $@"

$(CPP_ARTIFACT_DIR)/riftbridge_c.o: $(SRC_DIR)/riftbridge.c $(INC_DIR)/riftbridge.h | dirs
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

$(CPP_ARTIFACT_DIR)/riftbridge_cpp.o: $(SRC_DIR)/riftbridge.cpp $(INC_DIR)/riftbridge.hpp $(INC_DIR)/riftbridge.h | dirs
	$(CXX) $(CXXFLAGS) -fPIC -c -o $@ $<

# ============================================================================
# C# UI (requires .NET SDK)
# ============================================================================

csharp:
	@if command -v $(DOTNET) > /dev/null 2>&1; then \
		echo "[DOTNET] Building C# riftbridge UI..."; \
		$(DOTNET) build $(UI_DIR)/rift.csproj -o $(CSHARP_ARTIFACT_DIR) 2>/dev/null || \
		echo "[DOTNET] Note: Requires .NET project setup. See ui/ directory."; \
	else \
		echo "[SKIP] dotnet not found - skipping C# build"; \
	fi

run: ringboot
	@./$(RINGBOOT)

test: ringboot
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

debug: CFLAGS = $(DEBUG_CFLAGS)
debug: CXXFLAGS = $(DEBUG_CXXFLAGS)
debug: dirs ringboot
	@echo "[BUILD] Debug build complete"

clean:
	rm -rf $(ARTIFACTS_DIR)/c/mmuko-os $(ARTIFACTS_DIR)/cpp/mmuko-os $(CSHARP_ARTIFACT_DIR)
	@echo "[CLEAN] Build artifacts removed"

install: all bootimg riftbridge
	@echo "[INSTALL] Build artifacts:"
	@echo "  $(RINGBOOT)"
	@echo "  $(BOOTIMG)"
	@echo "  $(LIBRIFT_A)"
	@echo "  $(LIBRIFT_SO)"
