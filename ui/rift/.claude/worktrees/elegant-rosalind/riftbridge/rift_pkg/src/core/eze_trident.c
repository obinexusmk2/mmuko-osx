/* 
 * RIFT Tomography Core Implementation
 * Eze (Power) + Uche (Knowledge) + Obi (Heart/Soul)
 * 
 * Trident topology for diamond dependency resolution
 * Row/Column semantic intent via Iota matrix
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* ============================================================
 * CORE TYPE DEFINITIONS (from rift.h)
 * ============================================================ */

typedef enum {
    RIFT_POLARITY_POSITIVE = '+',
    RIFT_POLARITY_NEGATIVE = '-',
    RIFT_POLARITY_EPSILON  = '0'
} RiftPolarity;

typedef enum {
    SEMVER_LEGACY,
    SEMVER_EXPERIMENTAL,
    SEMVER_BETA,
    SEMVER_STABLE,
    SEMVER_LTS
} SemVerState;

typedef struct {
    uint32_t major;
    SemVerState major_state;
    uint32_t minor;
    SemVerState minor_state;
    uint32_t patch;
    SemVerState patch_state;
    RiftPolarity polarity;
} RiftVersion;

typedef struct RiftEzeNode {
    char name[64];
    RiftVersion version;
    struct RiftEzeNode *incoming[2];
    struct RiftEzeNode *outgoing;
    bool is_bound;
    RiftPolarity polarity;
} RiftEzeNode;

typedef struct {
    void *data;
    size_t size;
    RiftPolarity polarity;
} RiftUche;

typedef struct {
    RiftEzeNode *root;
    RiftUche *knowledge_base;
    RiftPolarity heartbeat;
} RiftObi;

typedef struct {
    uint32_t row;
    uint32_t col;
    RiftPolarity polarity;
    double confidence;
} IotaCell;

typedef struct {
    IotaCell **cells;
    uint32_t rows;
    uint32_t cols;
} IotaMatrix;

/* ============================================================
 * EZE (POWER/LEADER) - TRIDENT TOPOLOGY
 * ============================================================ */

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
    
    /* 2-of-2 Consensus: Both inputs must agree */
    if (!node->incoming[0] || !node->incoming[1]) {
        return false;
    }
    
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
        node->polarity = RIFT_POLARITY_NEGATIVE;
        return false;
    }
}

void rift_eze_destroy(RiftEzeNode *node) {
    if (node) free(node);
}

/* ============================================================
 * OBI (HEART/SOUL) - SEMVERX PARSER
 * ============================================================ */

static SemVerState parse_state(const char *state_str) {
    if (strcmp(state_str, "legacy") == 0) return SEMVER_LEGACY;
    if (strcmp(state_str, "experimental") == 0) return SEMVER_EXPERIMENTAL;
    if (strcmp(state_str, "beta") == 0) return SEMVER_BETA;
    if (strcmp(state_str, "stable") == 0) return SEMVER_STABLE;
    if (strcmp(state_str, "lts") == 0) return SEMVER_LTS;
    return SEMVER_STABLE;
}

static const char* state_to_string(SemVerState state) {
    switch (state) {
        case SEMVER_LEGACY: return "legacy";
        case SEMVER_EXPERIMENTAL: return "experimental";
        case SEMVER_BETA: return "beta";
        case SEMVER_STABLE: return "stable";
        case SEMVER_LTS: return "lts";
        default: return "unknown";
    }
}

RiftVersion rift_semver_parse(const char *version_string) {
    RiftVersion version = {0};
    char buffer[256];
    strncpy(buffer, version_string, sizeof(buffer) - 1);
    
    /* Format: major.state.minor.state.patch.state */
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

void rift_semver_print(RiftVersion v) {
    printf("%u.%s.%u.%s.%u.%s",
           v.major, state_to_string(v.major_state),
           v.minor, state_to_string(v.minor_state),
           v.patch, state_to_string(v.patch_state));
}

int rift_semver_compare(RiftVersion a, RiftVersion b) {
    if (a.major != b.major) return (int)(a.major - b.major);
    if (a.major_state != b.major_state) return (int)(a.major_state - b.major_state);
    
    if (a.minor != b.minor) return (int)(a.minor - b.minor);
    if (a.minor_state != b.minor_state) return (int)(a.minor_state - b.minor_state);
    
    if (a.patch != b.patch) return (int)(a.patch - b.patch);
    if (a.patch_state != b.patch_state) return (int)(a.patch_state - b.patch_state);
    
    return 0;
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
    free(obi);
}

/* ============================================================
 * IOTA (SHARED POWER) - ROW/COLUMN MATRIX
 * ============================================================ */

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

/* ============================================================
 * DEMONSTRATION: DIAMOND PROBLEM RESOLUTION
 * ============================================================ */

void demo_diamond_resolution(void) {
    printf("\n=== RIFT Trident Diamond Problem Demo ===\n\n");
    
    /* Scenario: Classic diamond dependency hell */
    RiftVersion v1_stable = rift_semver_parse("4.stable.17.beta.2.stable");
    RiftVersion v2_experimental = rift_semver_parse("4.experimental.17.beta.2.stable");
    RiftVersion v1_stable_dup = rift_semver_parse("4.stable.17.beta.2.stable");
    
    /* Create nodes */
    RiftEzeNode *dep_b = rift_eze_create("lodash", v1_stable);
    RiftEzeNode *dep_c = rift_eze_create("lodash", v2_experimental);
    RiftEzeNode *app = rift_eze_create("myapp", v1_stable);
    
    dep_b->polarity = RIFT_POLARITY_POSITIVE;
    dep_c->polarity = RIFT_POLARITY_POSITIVE;
    
    /* Connect trident: app depends on B and C */
    app->incoming[0] = dep_b;
    app->incoming[1] = dep_c;
    
    printf("[Test 1] Diamond Conflict Detection\n");
    printf("  B → lodash@");
    rift_semver_print(v1_stable);
    printf("\n  C → lodash@");
    rift_semver_print(v2_experimental);
    printf("\n\n");
    
    if (rift_eze_bind(app)) {
        printf("  Status: ✓ BOUND (consensus achieved)\n");
    } else {
        printf("  Status: ✗ UNBOUND (diamond conflict detected)\n");
        printf("  Polarity: %c (fault state)\n", app->polarity);
        printf("  → Application remains safe, no inconsistent state\n");
    }
    
    /* Scenario: Fix lands - hot-swap one input */
    printf("\n[Test 2] Hot-Swap Resolution\n");
    printf("  Publishing lodash@");
    rift_semver_print(v1_stable);
    printf(" to replace experimental...\n\n");
    
    /* Update dep_c to match */
    dep_c->version = v1_stable_dup;
    
    if (rift_eze_bind(app)) {
        printf("  Status: ✓ BOUND (hot-swap successful)\n");
        printf("  Polarity: %c (healthy state)\n", app->polarity);
        printf("  → Runtime healed without restart\n");
    } else {
        printf("  Status: ✗ UNBOUND (still conflicted)\n");
    }
    
    /* Cleanup */
    rift_eze_destroy(dep_b);
    rift_eze_destroy(dep_c);
    rift_eze_destroy(app);
    
    printf("\n=== Demo Complete ===\n");
}

void demo_iota_matrix(void) {
    printf("\n=== RIFT Iota Matrix Demo ===\n\n");
    
    /* Create 3x3 matrix for row/col semantic intent */
    IotaMatrix *matrix = iota_matrix_create(3, 3);
    
    printf("Matrix dimensions: %u rows × %u columns\n", matrix->rows, matrix->cols);
    printf("Row = temporal/statement sequence\n");
    printf("Col = structural depth/nesting level\n\n");
    
    /* Set some cells */
    iota_matrix_set(matrix, 0, 0, RIFT_POLARITY_POSITIVE, 0.95);
    iota_matrix_set(matrix, 0, 1, RIFT_POLARITY_POSITIVE, 0.85);
    iota_matrix_set(matrix, 1, 0, RIFT_POLARITY_NEGATIVE, 0.60);
    iota_matrix_set(matrix, 2, 2, RIFT_POLARITY_EPSILON, 0.50);
    
    printf("Matrix state:\n");
    for (uint32_t r = 0; r < matrix->rows; r++) {
        printf("  Row %u: ", r);
        for (uint32_t c = 0; c < matrix->cols; c++) {
            IotaCell cell = iota_matrix_get(matrix, r, c);
            printf("[%c:%.2f] ", cell.polarity, cell.confidence);
        }
        printf("\n");
    }
    
    iota_matrix_destroy(matrix);
    printf("\n=== Matrix Demo Complete ===\n");
}

/* ============================================================
 * MAIN ENTRY POINT
 * ============================================================ */

int main(void) {
    printf("RIFT Tomography System - C Implementation\n");
    printf("Eze (Power) + Uche (Knowledge) + Obi (Heart/Soul)\n");
    printf("OBINexus Constitutional Framework\n");
    
    /* Create Obi (central coordinator) */
    RiftObi *obi = rift_obi_create();
    printf("\n✓ Obi (Heart/Soul) initialized\n");
    printf("  Heartbeat: %c\n", obi->heartbeat);
    
    /* Run demonstrations */
    demo_diamond_resolution();
    demo_iota_matrix();
    
    /* Cleanup */
    rift_obi_destroy(obi);
    
    printf("\n✓ System shutdown complete\n");
    return 0;
}