# RIFT Tomography C-Family Implementation
## Trident SemVerX Package Manager for OBINexus

---

## Repository Structure

```
obinexus/rift/                    # Main tomography engine
├── include/
│   ├── rift/
│   │   ├── rift.h                # Core API
│   │   ├── riftbridge.h          # Bridge layer (all C-family)
│   │   └── riftest.h             # Testing framework
├── src/
│   ├── core/
│   │   ├── eze_trident.c         # Trident topology (power/leader)
│   │   ├── uche_resolver.c       # Knowledge resolver
│   │   └── obi_semver.c          # Heart/soul SemVerX
│   ├── bridge/
│   │   ├── c_bridge.c
│   │   ├── cpp_bridge.cpp
│   │   └── cs_bridge.cs
├── build/
│   ├── stage_c.sh                # C build script
│   ├── stage_cpp.sh              # C++ build script
│   └── stage_cs.sh               # C# build script
└── tests/

obinexus/riftbridge/              # Isolated package manager
├── include/
│   ├── riftbridge/
│   │   ├── iota_matrix.h         # Shared power matrix (row/col)
│   │   ├── polarity.h            # Polarity encoding
│   │   └── package.h             # Package definitions
├── src/
│   ├── registry/
│   │   ├── eze_registry.c        # Local registry (power node)
│   │   ├── uche_fetch.c          # Remote fetch (knowledge)
│   │   └── obi_resolve.c         # Dependency resolution
│   ├── encoding/
│   │   ├── rift_open.c           # Polyglot file opener
│   │   └── sparse_encode.c       # 2→1 duplex encoding
└── build/
    └── riftbridge_build.sh
```

---

## Core Header: `rift/rift.h`

```c
#ifndef RIFT_RIFT_H
#define RIFT_RIFT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MMUKO Spirit: Good/Evil Duality */
typedef enum {
    RIFT_POLARITY_POSITIVE = '+',
    RIFT_POLARITY_NEGATIVE = '-',
    RIFT_POLARITY_EPSILON  = '0'
} RiftPolarity;

/* SemVerX State (Constitutional Framework) */
typedef enum {
    SEMVER_LEGACY,
    SEMVER_EXPERIMENTAL,
    SEMVER_BETA,
    SEMVER_STABLE,
    SEMVER_LTS
} SemVerState;

/* SemVerX Version Structure */
typedef struct {
    uint32_t major;
    SemVerState major_state;
    uint32_t minor;
    SemVerState minor_state;
    uint32_t patch;
    SemVerState patch_state;
    RiftPolarity polarity;
} RiftVersion;

/* Trident Node (Eze = Power/Leader) */
typedef struct RiftEzeNode {
    char name[64];
    RiftVersion version;
    struct RiftEzeNode *incoming[2];  /* 2-of-2 consensus */
    struct RiftEzeNode *outgoing;     /* 1 propagation */
    bool is_bound;
    RiftPolarity polarity;
} RiftEzeNode;

/* Uche = Knowledge/Wisdom pointer */
typedef struct {
    void *data;
    size_t size;
    RiftPolarity polarity;
} RiftUche;

/* Obi = Heart/Soul pointer for resolution */
typedef struct {
    RiftEzeNode *root;
    RiftUche *knowledge_base;
    RiftPolarity heartbeat;
} RiftObi;

/* Core API */
RiftObi* rift_obi_create(void);
void rift_obi_destroy(RiftObi *obi);

RiftEzeNode* rift_eze_create(const char *name, RiftVersion version);
bool rift_eze_bind(RiftEzeNode *node);
bool rift_eze_consensus(RiftEzeNode *node);

RiftVersion rift_semver_parse(const char *version_string);
int rift_semver_compare(RiftVersion a, RiftVersion b);

#ifdef __cplusplus
}
#endif

#endif /* RIFT_RIFT_H */
```

---

## Bridge Header: `rift/riftbridge.h`

```c
#ifndef RIFT_RIFTBRIDGE_H
#define RIFT_RIFTBRIDGE_H

#include <rift/rift.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Iota Matrix: Shared Power (Row/Column Context) */
typedef struct {
    uint32_t row;        /* Statement/temporal sequence */
    uint32_t col;        /* Structural depth/nesting */
    RiftPolarity polarity;
    double confidence;   /* 0.0 to 1.0 */
} IotaCell;

typedef struct {
    IotaCell **cells;
    uint32_t rows;
    uint32_t cols;
} IotaMatrix;

/* Package Entry (Constitutional Compliance) */
typedef struct {
    char name[128];
    RiftVersion version;
    char tarball_hash[65];  /* SHA-256 hex */
    IotaMatrix *dependencies;
    RiftPolarity polarity;
} RiftPackage;

/* Registry Operations */
typedef struct {
    RiftPackage **packages;
    size_t count;
    size_t capacity;
    RiftObi *obi;  /* Heart/Soul resolver */
} RiftRegistry;

/* Bridge API */
RiftRegistry* rift_registry_create(void);
void rift_registry_destroy(RiftRegistry *reg);

bool rift_registry_add(RiftRegistry *reg, RiftPackage *pkg);
RiftPackage* rift_registry_find(RiftRegistry *reg, const char *name, RiftVersion version);
RiftPackage** rift_registry_resolve(RiftRegistry *reg, const char *name, size_t *out_count);

/* Iota Matrix Operations (Row/Col Semantic Intent) */
IotaMatrix* iota_matrix_create(uint32_t rows, uint32_t cols);
void iota_matrix_destroy(IotaMatrix *matrix);
bool iota_matrix_set(IotaMatrix *matrix, uint32_t row, uint32_t col, RiftPolarity polarity, double confidence);
IotaCell iota_matrix_get(IotaMatrix *matrix, uint32_t row, uint32_t col);

/* Polarity Encoding (RIFT Open) */
size_t rift_encode_sparse(const uint8_t *in, size_t in_len, uint8_t *out, RiftPolarity polarity);
size_t rift_decode_sparse(const uint8_t *in, size_t in_len, uint8_t *out, RiftPolarity polarity);

#ifdef __cplusplus
}
#endif

#endif /* RIFT_RIFTBRIDGE_H */
```

---

## Core Implementation: `src/core/eze_trident.c`

```c
#include <rift/rift.h>
#include <stdlib.h>
#include <string.h>

/* Eze = Power/Leader Trident Node */

RiftEzeNode* rift_eze_create(const char *name, RiftVersion version) {
    RiftEzeNode *node = calloc(1, sizeof(RiftEzeNode));
    if (!node) return NULL;
    
    strncpy(node->name, name, sizeof(node->name) - 1);
    node->version = version;
    node->incoming[0] = NULL;
    node->incoming[1] = NULL;
    node->outgoing = NULL;
    node->is_bound = false;
    node->polarity = RIFT_POLARITY_EPSILON;
    
    return node;
}

bool rift_eze_consensus(RiftEzeNode *node) {
    if (!node) return false;
    
    /* 2-of-2 Consensus Rule */
    if (!node->incoming[0] || !node->incoming[1]) {
        return false;  /* Need both inputs */
    }
    
    /* Check version identity */
    RiftVersion v1 = node->incoming[0]->version;
    RiftVersion v2 = node->incoming[1]->version;
    
    bool versions_match = (
        v1.major == v2.major &&
        v1.major_state == v2.major_state &&
        v1.minor == v2.minor &&
        v1.minor_state == v2.minor_state &&
        v1.patch == v2.patch &&
        v1.patch_state == v2.patch_state
    );
    
    /* Check polarity coherence */
    bool polarity_coherent = (
        node->incoming[0]->polarity == node->incoming[1]->polarity
    );
    
    return versions_match && polarity_coherent;
}

bool rift_eze_bind(RiftEzeNode *node) {
    if (!node) return false;
    
    if (rift_eze_consensus(node)) {
        node->is_bound = true;
        node->version = node->incoming[0]->version;
        node->polarity = node->incoming[0]->polarity;
        return true;
    } else {
        node->is_bound = false;
        node->polarity = RIFT_POLARITY_NEGATIVE;  /* Fault state */
        return false;
    }
}
```

---

## Core Implementation: `src/core/obi_semver.c`

```c
#include <rift/rift.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Obi = Heart/Soul SemVerX Parser */

static SemVerState parse_state(const char *state_str) {
    if (strcmp(state_str, "legacy") == 0) return SEMVER_LEGACY;
    if (strcmp(state_str, "experimental") == 0) return SEMVER_EXPERIMENTAL;
    if (strcmp(state_str, "beta") == 0) return SEMVER_BETA;
    if (strcmp(state_str, "stable") == 0) return SEMVER_STABLE;
    if (strcmp(state_str, "lts") == 0) return SEMVER_LTS;
    return SEMVER_STABLE;  /* Default */
}

RiftVersion rift_semver_parse(const char *version_string) {
    RiftVersion version = {0};
    char buffer[256];
    strncpy(buffer, version_string, sizeof(buffer) - 1);
    
    /* Format: major.state.minor.state.patch.state */
    /* Example: 4.stable.17.beta.2.stable */
    
    char *tokens[6];
    int count = 0;
    char *token = strtok(buffer, ".");
    
    while (token && count < 6) {
        tokens[count++] = token;
        token = strtok(NULL, ".");
    }
    
    if (count >= 2) {
        version.major = (uint32_t)atoi(tokens[0]);
        version.major_state = parse_state(tokens[1]);
    }
    if (count >= 4) {
        version.minor = (uint32_t)atoi(tokens[2]);
        version.minor_state = parse_state(tokens[3]);
    }
    if (count >= 6) {
        version.patch = (uint32_t)atoi(tokens[4]);
        version.patch_state = parse_state(tokens[5]);
    }
    
    version.polarity = RIFT_POLARITY_POSITIVE;
    return version;
}

int rift_semver_compare(RiftVersion a, RiftVersion b) {
    /* Major comparison */
    if (a.major != b.major) return (int)(a.major - b.major);
    if (a.major_state != b.major_state) return (int)(a.major_state - b.major_state);
    
    /* Minor comparison */
    if (a.minor != b.minor) return (int)(a.minor - b.minor);
    if (a.minor_state != b.minor_state) return (int)(a.minor_state - b.minor_state);
    
    /* Patch comparison */
    if (a.patch != b.patch) return (int)(a.patch - b.patch);
    if (a.patch_state != b.patch_state) return (int)(a.patch_state - b.patch_state);
    
    return 0;  /* Equal */
}

RiftObi* rift_obi_create(void) {
    RiftObi *obi = calloc(1, sizeof(RiftObi));
    if (!obi) return NULL;
    
    obi->root = NULL;
    obi->knowledge_base = NULL;
    obi->heartbeat = RIFT_POLARITY_POSITIVE;
    
    return obi;
}

void rift_obi_destroy(RiftObi *obi) {
    if (!obi) return;
    /* Recursive cleanup would go here */
    free(obi);
}
```

---

## Bridge Implementation: `src/bridge/iota_matrix.c`

```c
#include <rift/riftbridge.h>
#include <stdlib.h>
#include <string.h>

/* Iota = Shared Power Matrix (Row/Col Context) */

IotaMatrix* iota_matrix_create(uint32_t rows, uint32_t cols) {
    IotaMatrix *matrix = malloc(sizeof(IotaMatrix));
    if (!matrix) return NULL;
    
    matrix->rows = rows;
    matrix->cols = cols;
    matrix->cells = calloc(rows, sizeof(IotaCell*));
    
    for (uint32_t i = 0; i < rows; i++) {
        matrix->cells[i] = calloc(cols, sizeof(IotaCell));
        for (uint32_t j = 0; j < cols; j++) {
            matrix->cells[i][j].row = i;
            matrix->cells[i][j].col = j;
            matrix->cells[i][j].polarity = RIFT_POLARITY_EPSILON;
            matrix->cells[i][j].confidence = 0.0;
        }
    }
    
    return matrix;
}

void iota_matrix_destroy(IotaMatrix *matrix) {
    if (!matrix) return;
    
    for (uint32_t i = 0; i < matrix->rows; i++) {
        free(matrix->cells[i]);
    }
    free(matrix->cells);
    free(matrix);
}

bool iota_matrix_set(IotaMatrix *matrix, uint32_t row, uint32_t col, 
                     RiftPolarity polarity, double confidence) {
    if (!matrix || row >= matrix->rows || col >= matrix->cols) return false;
    
    matrix->cells[row][col].polarity = polarity;
    matrix->cells[row][col].confidence = confidence;
    return true;
}

IotaCell iota_matrix_get(IotaMatrix *matrix, uint32_t row, uint32_t col) {
    IotaCell empty = {0, 0, RIFT_POLARITY_EPSILON, 0.0};
    if (!matrix || row >= matrix->rows || col >= matrix->cols) return empty;
    
    return matrix->cells[row][col];
}
```

---

## Build Script: `build/stage_c.sh`

```bash
#!/bin/bash
# RIFT Tomography C Build Script (Stage-1)

set -e

echo "=== RIFT C Build Stage ==="
echo "Building Eze (Power), Uche (Knowledge), Obi (Heart/Soul)..."

CC=${CC:-gcc}
CFLAGS="-std=c11 -O2 -Wall -Wextra -Iinclude"
LDFLAGS="-lm"

mkdir -p build/obj build/lib build/bin

# Core objects
echo "[1/6] Building eze_trident.c..."
$CC $CFLAGS -c src/core/eze_trident.c -o build/obj/eze_trident.o

echo "[2/6] Building uche_resolver.c..."
$CC $CFLAGS -c src/core/uche_resolver.c -o build/obj/uche_resolver.o

echo "[3/6] Building obi_semver.c..."
$CC $CFLAGS -c src/core/obi_semver.c -o build/obj/obi_semver.o

# Bridge objects
echo "[4/6] Building iota_matrix.c..."
$CC $CFLAGS -c src/bridge/iota_matrix.c -o build/obj/iota_matrix.o

echo "[5/6] Building rift_open.c..."
$CC $CFLAGS -c src/encoding/rift_open.c -o build/obj/rift_open.o

# Link library
echo "[6/6] Linking librift.a..."
ar rcs build/lib/librift.a build/obj/*.o

echo "✓ RIFT C library built: build/lib/librift.a"
echo "=== Build Complete ==="
```

---

## Usage Guide

### Step 1: Clone Repositories

```bash
# Main tomography engine
git clone https://github.com/obinexus/rift.git
cd rift

# Package manager bridge
cd ..
git clone https://github.com/obinexus/riftbridge.git
cd riftbridge
```

### Step 2: Build RIFT Core

```bash
cd rift
chmod +x build/stage_c.sh
./build/stage_c.sh
```

### Step 3: Build RIFTBridge

```bash
cd ../riftbridge
chmod +x build/riftbridge_build.sh
./build/riftbridge_build.sh
```

### Step 4: Example Usage

```c
#include <rift/riftbridge.h>
#include <stdio.h>

int main(void) {
    /* Create Obi (Heart/Soul resolver) */
    RiftObi *obi = rift_obi_create();
    
    /* Create Eze nodes (Trident topology) */
    RiftVersion v1 = rift_semver_parse("4.stable.17.beta.2.stable");
    RiftVersion v2 = rift_semver_parse("4.stable.17.beta.2.stable");
    
    RiftEzeNode *node_a = rift_eze_create("lodash", v1);
    RiftEzeNode *node_b = rift_eze_create("lodash", v2);
    RiftEzeNode *app = rift_eze_create("myapp", v1);
    
    /* Connect trident */
    app->incoming[0] = node_a;
    app->incoming[1] = node_b;
    
    /* Test consensus */
    if (rift_eze_bind(app)) {
        printf("✓ Trident bound: consensus achieved\n");
    } else {
        printf("✗ Trident unbound: diamond conflict detected\n");
    }
    
    /* Cleanup */
    rift_obi_destroy(obi);
    free(node_a);
    free(node_b);
    free(app);
    
    return 0;
}
```

### Step 5: Compile Your Program

```bash
gcc -Irift/include -Iriftbridge/include \
    myapp.c \
    -Lrift/build/lib -Lriftbridge/build/lib \
    -lrift -lriftbridge \
    -o myapp

./myapp
```

---

## Key Concepts

### Eze (Power/Leader)
- Trident node with 2 incoming + 1 outgoing
- Enforces 2-of-2 consensus
- Diamond problem isolation

### Uche (Knowledge/Wisdom)
- Resolver logic
- Remote fetch capabilities
- Learning from past resolutions

### Obi (Heart/Soul)
- Central coordination
- Polarity tracking (good/evil duality)
- System heartbeat monitoring

### Iota (Shared Power)
- Row = temporal/statement sequence
- Column = structural depth/nesting
- Polarity encoding per cell

---

## Constitutional Compliance

This implementation follows OBINexus NT License v1.0:
- ✓ Human rights respect (no harassment)
- ✓ Constructive engagement (patch-driven)
- ✓ OpenSense sustainability model
- ✓ Automated governance ready

---

**For what is yet to be, I became.**  
— Nnamdi Okpala, OBINexus Computing
