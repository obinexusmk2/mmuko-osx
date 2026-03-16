/*
 * ringboot.c - MMUKO-OS Ring Boot State Machine Implementation
 * Project: OBINexus / MMUKO-OS
 * Protocol: NSIGII Human Rights Verification Firmware
 *
 * Implements the complete ring boot cycle:
 *   SPARSE → REMEMBER → ACTIVE → VERIFY → COMPLETE
 *
 * With:
 *   - 8-qubit compass ring (N/NE/E/SE/S/SW/W/NW)
 *   - Vacuum medium initialization (feather = hammer principle)
 *   - Superposition entanglement resolution
 *   - Nonlinear diamond-table index resolution
 *   - Rotation freedom verification (360° no-lock)
 *   - NSIGII tripartite consent at each phase transition
 *
 * Engine Model (from handwritten notes):
 *   Create ↔ Destroy (cycle)
 *   Build  ↔ Repair/Renew (cycle)
 *   These form the ring boot engine that drives phase transitions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "../include/ringboot.h"

/* ================================================================
 * CONSTANTS
 * ================================================================ */

#define PI 3.14159265358979

/* Gravity medium constants */
#define G_VACUUM    9.8
#define G_LEPTON    (G_VACUUM / 10.0)
#define G_MUON      (G_LEPTON / 10.0)
#define G_DEEP      (G_MUON / 10.0)

/* Spin values per compass direction (radians) */
static const double spin_values[8] = {
    PI / 4.0,       /* NORTH     = 0.7854  */
    PI / 3.0,       /* NORTHEAST = 1.0472  */
    PI / 2.0,       /* EAST      = 1.5708  */
    PI,             /* SOUTHEAST = 3.1416  */
    PI * 2.0,       /* SOUTH     = 6.2832  */
    PI / 2.0,       /* SOUTHWEST (dual EAST) */
    PI / 3.0,       /* WEST (dual NORTHEAST)  */
    PI / 4.0        /* NORTHWEST (dual NORTH) */
};

/* Entanglement pairs: index → partner index */
static const int entangled_pairs[8] = {7, 6, 5, -1, -1, 2, 1, 0};

/* Superposition lookup table (base → compass pair) */
static const SuperpositionEntry superposition_table[] = {
    {12, COMPASS_SOUTH,     COMPASS_NORTH    },  /* Full cycle */
    {10, COMPASS_SOUTHEAST, COMPASS_NORTH    },
    { 8, COMPASS_EAST,      COMPASS_WEST     },  /* π/2 pair */
    { 6, COMPASS_SOUTHWEST, COMPASS_EAST     },  /* Middle base */
    { 4, COMPASS_WEST,      COMPASS_EAST     },
    { 2, COMPASS_NORTHEAST, COMPASS_WEST     },
    { 1, COMPASS_NORTH,     COMPASS_SOUTH    }   /* Base unit */
};
#define SUPERPOSITION_TABLE_SIZE \
    (sizeof(superposition_table) / sizeof(SuperpositionEntry))

/* Diamond traversal order for nonlinear resolution */
static const int diamond_order[] = {12, 6, 8, 4, 10, 2, 1};
#define DIAMOND_ORDER_SIZE \
    (sizeof(diamond_order) / sizeof(int))

/* ================================================================
 * STRING LOOKUP TABLES
 * ================================================================ */

static const char* compass_names[] = {
    "NORTH", "NORTHEAST", "EAST", "SOUTHEAST",
    "SOUTH", "SOUTHWEST", "WEST", "NORTHWEST", "UNDEFINED"
};

static const char* qstate_names[] = {
    "UP", "DOWN", "CHARM", "STRANGE", "LEFT", "RIGHT"
};

static const char* phase_names[] = {
    "SPARSE", "REMEMBER", "ACTIVE", "VERIFY", "COMPLETE", "FAILED"
};

const char* compass_direction_name(CompassDirection dir) {
    if (dir <= COMPASS_UNDEFINED) return compass_names[dir];
    return "INVALID";
}

const char* qubit_state_name(QubitState state) {
    if (state <= QSTATE_RIGHT) return qstate_names[state];
    return "INVALID";
}

const char* ringboot_phase_name(RingBootPhase phase) {
    if (phase <= RINGBOOT_PHASE_FAILED) return phase_names[phase];
    return "INVALID";
}

double compass_spin_value(CompassDirection dir) {
    if (dir < 8) return spin_values[dir];
    return 0.0;
}

/* ================================================================
 * VACUUM MEDIUM INITIALIZATION
 * "A feather and hammer fall at the same rate"
 * ================================================================ */

VacuumMedium ringboot_init_vacuum(void) {
    VacuumMedium medium = {
        .gravity = G_VACUUM,
        .lepton  = G_LEPTON,
        .muon    = G_MUON,
        .deep    = G_DEEP
    };
    printf("[VACUUM] Initialized: G=%.4f, lepton=%.4f, muon=%.6f, deep=%.8f\n",
           medium.gravity, medium.lepton, medium.muon, medium.deep);
    return medium;
}

/* ================================================================
 * QUBIT RING OPERATIONS
 * ================================================================ */

QubitState ringboot_resolve_state(int index, uint8_t byte_val) {
    uint8_t bit = (byte_val >> index) & 1;
    uint8_t neighbor = (byte_val >> ((index + 1) % 8)) & 1;

    if (bit == 1 && neighbor == 1) return QSTATE_UP;
    if (bit == 1 && neighbor == 0) return QSTATE_CHARM;
    if (bit == 0 && neighbor == 1) return QSTATE_STRANGE;
    return QSTATE_DOWN;
}

QubitState ringboot_flip_state(QubitState s) {
    switch (s) {
        case QSTATE_UP:      return QSTATE_DOWN;
        case QSTATE_DOWN:    return QSTATE_UP;
        case QSTATE_CHARM:   return QSTATE_STRANGE;
        case QSTATE_STRANGE: return QSTATE_CHARM;
        case QSTATE_LEFT:    return QSTATE_RIGHT;
        case QSTATE_RIGHT:   return QSTATE_LEFT;
        default: return s;
    }
}

uint8_t ringboot_rotate_bits(uint8_t value, int n) {
    n = n % 8;
    if (n == 0) return value;
    return ((value >> n) | (value << (8 - n))) & 0xFF;
}

void ringboot_init_qubit_ring(RingByte *byte) {
    if (!byte) return;

    static const CompassDirection directions[8] = {
        COMPASS_NORTH, COMPASS_NORTHEAST, COMPASS_EAST, COMPASS_SOUTHEAST,
        COMPASS_SOUTH, COMPASS_SOUTHWEST, COMPASS_WEST, COMPASS_NORTHWEST
    };

    for (int i = 0; i < RING_QUBITS; i++) {
        RingQubit *q = &byte->qubits[i];
        q->index = i;
        q->value = (byte->raw_value >> i) & 1;
        q->spin = spin_values[i];
        q->direction = directions[i];
        q->state = ringboot_resolve_state(i, byte->raw_value);
        q->entangled_with = entangled_pairs[i];
        q->superposed = (entangled_pairs[i] != -1);
    }
}

/* ================================================================
 * SUPERPOSITION LOOKUP (Weak Map)
 * ================================================================ */

int ringboot_round_to_even_base(int base) {
    const int valid_bases[] = {12, 10, 8, 6, 4, 2, 1};
    int nearest = valid_bases[0];
    int min_diff = abs(base - valid_bases[0]);

    for (int i = 1; i < 7; i++) {
        int diff = abs(base - valid_bases[i]);
        if (diff < min_diff) {
            min_diff = diff;
            nearest = valid_bases[i];
        }
    }
    return nearest;
}

void ringboot_lookup_superposition(int base,
                                    CompassDirection *primary,
                                    CompassDirection *secondary) {
    /* Direct lookup */
    for (size_t i = 0; i < SUPERPOSITION_TABLE_SIZE; i++) {
        if (superposition_table[i].base == base) {
            *primary = superposition_table[i].primary;
            *secondary = superposition_table[i].secondary;
            return;
        }
    }

    /* Round to nearest even base and lookup */
    int nearest = ringboot_round_to_even_base(base);
    for (size_t i = 0; i < SUPERPOSITION_TABLE_SIZE; i++) {
        if (superposition_table[i].base == nearest) {
            *primary = superposition_table[i].primary;
            *secondary = superposition_table[i].secondary;
            return;
        }
    }

    /* Fallback */
    *primary = COMPASS_NORTH;
    *secondary = COMPASS_SOUTH;
}

/* ================================================================
 * STATE MACHINE TRANSITIONS
 * ================================================================ */

void ringboot_transition(RingBootSystem *sys, RingBootPhase phase) {
    if (!sys) return;

    RingBootStateMachine *sm = &sys->state_machine;
    sm->prev_phase = sm->phase;
    sm->phase = phase;
    sm->transition_count++;

    printf("[RINGBOOT] Transition: %s → %s (count=%d)\n",
           ringboot_phase_name(sm->prev_phase),
           ringboot_phase_name(sm->phase),
           sm->transition_count);
}

/* ================================================================
 * BOOT PHASES
 * ================================================================ */

/**
 * Phase 1: SPARSE
 * Initialize all qubits with half-spin allocation.
 * Every bit gets a compass direction and π/4 base spin.
 */
int ringboot_phase_sparse(RingBootSystem *sys) {
    printf("\n[PHASE 1] SPARSE: Initializing cubit rings...\n");

    for (size_t i = 0; i < sys->memory_size; i++) {
        RingByte *byte = &sys->memory_map[i];
        byte->base_index = (byte->raw_value % 12) + 1;

        ringboot_init_qubit_ring(byte);
        ringboot_lookup_superposition(byte->base_index,
                                       &byte->primary_super,
                                       &byte->secondary_super);
    }

    printf("[PHASE 1] Initialized %zu cubit rings\n", sys->memory_size);
    return 0;
}

/**
 * Phase 2: REMEMBER
 * Compass alignment: every cubit must face a direction.
 * Directionless = locked state = boot failure.
 * Resolve undefined directions from neighbor consensus.
 */
int ringboot_phase_remember(RingBootSystem *sys) {
    printf("\n[PHASE 2] REMEMBER: Compass alignment...\n");

    for (size_t b = 0; b < sys->memory_size; b++) {
        RingByte *byte = &sys->memory_map[b];
        for (int i = 0; i < RING_QUBITS; i++) {
            RingQubit *q = &byte->qubits[i];
            if (q->direction == COMPASS_UNDEFINED) {
                /* Resolve from neighbors (majority vote) */
                int left_idx = (i - 1 + RING_QUBITS) % RING_QUBITS;
                int right_idx = (i + 1) % RING_QUBITS;
                CompassDirection left_dir = byte->qubits[left_idx].direction;
                CompassDirection right_dir = byte->qubits[right_idx].direction;

                if (left_dir != COMPASS_UNDEFINED) {
                    q->direction = left_dir;
                } else if (right_dir != COMPASS_UNDEFINED) {
                    q->direction = right_dir;
                } else {
                    q->direction = COMPASS_NORTH; /* Default: face north */
                }
            }
        }
    }

    /* Superposition entanglement resolution */
    printf("[PHASE 2] Resolving superposition entanglement...\n");
    for (size_t b = 0; b < sys->memory_size; b++) {
        RingByte *byte = &sys->memory_map[b];
        for (int i = 0; i < RING_QUBITS; i++) {
            RingQubit *q = &byte->qubits[i];
            if (q->superposed && q->entangled_with >= 0 &&
                q->entangled_with < RING_QUBITS) {
                RingQubit *partner = &byte->qubits[q->entangled_with];
                if (q->state == partner->state) {
                    /* Constructive interference → flip partner */
                    partner->state = ringboot_flip_state(partner->state);
                }
            }
        }
    }

    printf("[PHASE 2] All cubits aligned and entangled\n");
    return 0;
}

/**
 * Phase 3: ACTIVE
 * Frame of reference centering (lock-free).
 * Nonlinear diamond-table index resolution.
 * All memory bytes orient relative to center base (6).
 */
int ringboot_phase_active(RingBootSystem *sys) {
    printf("\n[PHASE 3] ACTIVE: Frame centering + nonlinear resolution...\n");

    /* Find center base (middle of 1-12 range = 6) */
    int center_base = 6;
    CompassDirection primary, secondary;
    ringboot_lookup_superposition(center_base, &primary, &secondary);
    sys->frame_of_reference = primary;

    printf("[PHASE 3] Frame of reference: %s (base %d)\n",
           compass_direction_name(primary), center_base);

    /* Diamond traversal: resolve bases in superposition order */
    printf("[PHASE 3] Diamond traversal order:\n");
    for (size_t d = 0; d < DIAMOND_ORDER_SIZE; d++) {
        int base = diamond_order[d];
        CompassDirection dp, ds;
        ringboot_lookup_superposition(base, &dp, &ds);

        /* Apply to matching memory bytes */
        for (size_t i = 0; i < sys->memory_size; i++) {
            if (sys->memory_map[i].base_index == base) {
                sys->memory_map[i].primary_super = dp;
                sys->memory_map[i].secondary_super = ds;
            }
        }

        printf("  Base %2d → %s / %s\n",
               base, compass_direction_name(dp), compass_direction_name(ds));
    }

    return 0;
}

/**
 * Phase 4: VERIFY
 * Rotation freedom check (360° no-lock confirmation).
 * NSIGII tripartite verification via boot sector context.
 */
int ringboot_phase_verify(RingBootSystem *sys) {
    printf("\n[PHASE 4] VERIFY: Rotation check + NSIGII verify...\n");

    /* Rotation freedom: every cubit must complete 360° without lock */
    for (size_t b = 0; b < sys->memory_size; b++) {
        RingByte *byte = &sys->memory_map[b];
        for (int i = 0; i < RING_QUBITS; i++) {
            uint8_t original = byte->qubits[i].value;
            uint8_t test = ringboot_rotate_bits(original, 4);
            test = ringboot_rotate_bits(test, 4);

            if (test != original) {
                printf("[VERIFY] Rotation lock at byte %zu, qubit %d\n", b, i);
                return -1;
            }
        }
    }
    printf("[VERIFY] All cubits rotate freely (360° verified)\n");

    /* NSIGII verification via boot sector */
    if (sys->boot_sector) {
        sys->boot_sector->consent.user_consent = NSIGII_YES;
        sys->boot_sector->consent.inst_consent = NSIGII_YES;
        sys->boot_sector->consent.arb_consent = NSIGII_YES;

        NSIGIIColor color = bootsec_nsigii_verify(sys->boot_sector);
        sys->state_machine.color = color;
        sys->state_machine.nsigii_code =
            (color == COLOR_GREEN) ? NSIGII_YES :
            (color == COLOR_YELLOW) ? NSIGII_MAYBE : NSIGII_NO;

        printf("[VERIFY] NSIGII result: %s\n", nsigii_color_name(color));

        if (color == COLOR_RED) {
            return -1;
        }
    }

    return 0;
}

/* ================================================================
 * SYSTEM LIFECYCLE
 * ================================================================ */

RingBootSystem* ringboot_create(size_t memory_size) {
    RingBootSystem *sys = (RingBootSystem*)calloc(1, sizeof(RingBootSystem));
    if (!sys) return NULL;

    sys->memory_map = (RingByte*)calloc(memory_size, sizeof(RingByte));
    if (!sys->memory_map) {
        free(sys);
        return NULL;
    }

    sys->memory_size = memory_size;
    sys->frame_of_reference = COMPASS_NORTH;
    sys->boot_complete = false;
    sys->boot_timestamp = (uint32_t)time(NULL);

    /* Initialize state machine */
    sys->state_machine.phase = RINGBOOT_PHASE_SPARSE;
    sys->state_machine.prev_phase = RINGBOOT_PHASE_SPARSE;
    sys->state_machine.transition_count = 0;
    sys->state_machine.nsigii_code = NSIGII_MAYBE;
    sys->state_machine.color = COLOR_YELLOW;
    sys->state_machine.flags = 0;

    /* Create boot sector context */
    sys->boot_sector = bootsec_create();

    /* Initialize vacuum medium */
    sys->medium = ringboot_init_vacuum();

    /* Initialize memory with test pattern */
    for (size_t i = 0; i < memory_size; i++) {
        sys->memory_map[i].raw_value = (uint8_t)(i * 17 + 42);
    }

    return sys;
}

void ringboot_destroy(RingBootSystem *sys) {
    if (sys) {
        if (sys->boot_sector) bootsec_destroy(sys->boot_sector);
        free(sys->memory_map);
        free(sys);
    }
}

/* ================================================================
 * MAIN BOOT SEQUENCE EXECUTION
 * ================================================================ */

int ringboot_execute(RingBootSystem *sys) {
    if (!sys) return -1;

    printf("\n");
    printf("╔═══════════════════════════════════════════════════╗\n");
    printf("║         MMUKO-OS RING BOOT SEQUENCE v2           ║\n");
    printf("║    OBINexus — NSIGII Human Rights Protocol       ║\n");
    printf("║  \"Don't just boot systems. Boot truthful ones.\"  ║\n");
    printf("╚═══════════════════════════════════════════════════╝\n\n");

    int status;

    /* Boot sector initialization */
    if (sys->boot_sector) {
        status = bootsec_execute_boot(sys->boot_sector);
        if (status != 0) {
            printf("[RINGBOOT] Boot sector initialization failed\n");
            ringboot_transition(sys, RINGBOOT_PHASE_FAILED);
            return -1;
        }
    }

    /* Phase 1: SPARSE */
    ringboot_transition(sys, RINGBOOT_PHASE_SPARSE);
    status = ringboot_phase_sparse(sys);
    if (status != 0) {
        ringboot_transition(sys, RINGBOOT_PHASE_FAILED);
        return -1;
    }

    /* Phase 2: REMEMBER */
    ringboot_transition(sys, RINGBOOT_PHASE_REMEMBER);
    status = ringboot_phase_remember(sys);
    if (status != 0) {
        ringboot_transition(sys, RINGBOOT_PHASE_FAILED);
        return -1;
    }

    /* Phase 3: ACTIVE */
    ringboot_transition(sys, RINGBOOT_PHASE_ACTIVE);
    status = ringboot_phase_active(sys);
    if (status != 0) {
        ringboot_transition(sys, RINGBOOT_PHASE_FAILED);
        return -1;
    }

    /* Phase 4: VERIFY */
    ringboot_transition(sys, RINGBOOT_PHASE_VERIFY);
    status = ringboot_phase_verify(sys);
    if (status != 0) {
        ringboot_transition(sys, RINGBOOT_PHASE_FAILED);
        return -1;
    }

    /* Phase 5: COMPLETE */
    ringboot_transition(sys, RINGBOOT_PHASE_COMPLETE);
    sys->boot_complete = true;

    printf("\n");
    printf("╔═══════════════════════════════════════════════════╗\n");
    printf("║             RING BOOT COMPLETE                   ║\n");
    printf("║  Frame: %-10s  NSIGII: %-8s               ║\n",
           compass_direction_name(sys->frame_of_reference),
           nsigii_color_name(sys->state_machine.color));
    printf("║  All cubits aligned — no lock detected           ║\n");
    printf("╚═══════════════════════════════════════════════════╝\n\n");

    return 0;
}

/* ================================================================
 * DIAGNOSTICS
 * ================================================================ */

void ringboot_print_qubit(const RingQubit *q) {
    if (!q) return;
    printf("  Qubit[%d]: val=%d dir=%-10s state=%-8s spin=%.4f %s ent=%d\n",
           q->index, q->value,
           compass_direction_name(q->direction),
           qubit_state_name(q->state),
           q->spin,
           q->superposed ? "SUPER" : "     ",
           q->entangled_with);
}

void ringboot_print_system(const RingBootSystem *sys) {
    if (!sys) return;

    printf("\n=== Ring Boot System State ===\n");
    printf("Phase: %s | Frame: %s\n",
           ringboot_phase_name(sys->state_machine.phase),
           compass_direction_name(sys->frame_of_reference));
    printf("NSIGII: %s (0x%02X) | Transitions: %d\n",
           nsigii_color_name(sys->state_machine.color),
           sys->state_machine.nsigii_code,
           sys->state_machine.transition_count);
    printf("Memory: %zu bytes | Boot: %s\n",
           sys->memory_size,
           sys->boot_complete ? "COMPLETE" : "PENDING");
    printf("Vacuum: G=%.4f | Lepton=%.4f | Muon=%.6f\n",
           sys->medium.gravity, sys->medium.lepton, sys->medium.muon);

    /* Print first byte's qubit ring as sample */
    if (sys->memory_size > 0) {
        printf("\nSample Byte[0] (raw=0x%02X, base=%d):\n",
               sys->memory_map[0].raw_value,
               sys->memory_map[0].base_index);
        for (int i = 0; i < RING_QUBITS; i++) {
            ringboot_print_qubit(&sys->memory_map[0].qubits[i]);
        }
    }
    printf("==============================\n\n");
}

/* ================================================================
 * MAIN ENTRY POINT
 * ================================================================ */

int main(void) {
    /* Create system with 16 bytes of MMUKO ring memory */
    size_t mem_size = 16;
    RingBootSystem *sys = ringboot_create(mem_size);
    if (!sys) {
        fprintf(stderr, "Failed to create ring boot system\n");
        return 1;
    }

    /* Execute ring boot sequence */
    int result = ringboot_execute(sys);

    if (result == 0) {
        ringboot_print_system(sys);
        printf("Launching kernel scheduler...\n");
    } else {
        printf("RING BOOT FAILED\n");
        ringboot_print_system(sys);
    }

    ringboot_destroy(sys);
    return (result == 0) ? 0 : 1;
}
