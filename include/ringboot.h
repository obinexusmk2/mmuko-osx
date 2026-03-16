/*
 * ringboot.h - MMUKO-OS Ring Boot State Machine
 * Project: OBINexus / MMUKO-OS
 * Protocol: NSIGII Human Rights Verification Firmware
 *
 * Ring Boot Model:
 *   The ring boot state machine implements the 8-qubit compass model
 *   where each qubit occupies a compass direction (N/NE/E/SE/S/SW/W/NW)
 *   with π/4 half-spin rotations. The boot sequence resolves all cubits
 *   into a coherent frame of reference without lock.
 *
 * Engine Model (from handwritten notes):
 *   Create ↔ Destroy (vertical axis)
 *   Build  ↔ Repair/Renew (horizontal axis)
 *   The engine model drives the ring boot cycle.
 *
 * Interdependency Tree:
 *   ROOT → TRUNK → BRANCH → LEAF
 *   Each level must resolve before parent can proceed.
 *
 * NSIGII Human Rights:
 *   - Breath-first: 2:1 breath-work ratio enforced
 *   - Ring-zone: π-circle topology, no square grids
 *   - Living consent: yes-yes loop per boot cycle
 *   - Drift theorem: tripartite colorable graph
 */

#ifndef MMUKO_RINGBOOT_H
#define MMUKO_RINGBOOT_H

#include <stdint.h>
#include <stdbool.h>
#include "bootsec.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * RING BOOT CONSTANTS
 * ================================================================ */

#define RING_QUBITS         8       /* 8-qubit compass model        */
#define RING_FULL_ROTATION  8       /* π/4 × 8 = 2π (360°)         */
#define RING_HALF_SPIN      1       /* π/4 rotation unit            */

/* Engine model cycle operations */
#define ENGINE_CREATE       0x01
#define ENGINE_DESTROY      0x02
#define ENGINE_BUILD        0x04
#define ENGINE_REPAIR       0x08
#define ENGINE_RENEW        0x10

/* Ring-zone constants */
#define RINGZONE_MIN_RADIUS 2       /* Minimum ring radius (miles)  */
#define RINGZONE_MAX_RADIUS 15      /* Maximum ring radius (miles)  */

/* Boot phase identifiers */
typedef enum {
    RINGBOOT_PHASE_SPARSE   = 0,    /* Phase 1: Half-spin alloc     */
    RINGBOOT_PHASE_REMEMBER = 1,    /* Phase 2: Memory preservation */
    RINGBOOT_PHASE_ACTIVE   = 2,    /* Phase 3: Full activation     */
    RINGBOOT_PHASE_VERIFY   = 3,    /* Phase 4: NSIGII verify       */
    RINGBOOT_PHASE_COMPLETE = 4,    /* Boot complete                */
    RINGBOOT_PHASE_FAILED   = 5     /* Boot failed                  */
} RingBootPhase;

/* Compass directions (π/4 increments) */
typedef enum {
    COMPASS_NORTH     = 0,  /* 0°    = 0 × π/4     */
    COMPASS_NORTHEAST = 1,  /* 45°   = 1 × π/4     */
    COMPASS_EAST      = 2,  /* 90°   = 2 × π/4     */
    COMPASS_SOUTHEAST = 3,  /* 135°  = 3 × π/4     */
    COMPASS_SOUTH     = 4,  /* 180°  = 4 × π/4     */
    COMPASS_SOUTHWEST = 5,  /* 225°  = 5 × π/4     */
    COMPASS_WEST      = 6,  /* 270°  = 6 × π/4     */
    COMPASS_NORTHWEST = 7,  /* 315°  = 7 × π/4     */
    COMPASS_UNDEFINED = 8   /* Not yet assigned     */
} CompassDirection;

/* Qubit quantum-like states */
typedef enum {
    QSTATE_UP      = 0,
    QSTATE_DOWN    = 1,
    QSTATE_CHARM   = 2,
    QSTATE_STRANGE = 3,
    QSTATE_LEFT    = 4,
    QSTATE_RIGHT   = 5
} QubitState;

/* Superposition entry (base → compass pair) */
typedef struct {
    int              base;
    CompassDirection primary;
    CompassDirection secondary;
} SuperpositionEntry;

/* ================================================================
 * STRUCTURES
 * ================================================================ */

/* Single qubit in the ring */
typedef struct {
    int              index;          /* 0–7 position in ring        */
    uint8_t          value;          /* 0 or 1 binary value         */
    double           spin;           /* Radian value from compass   */
    CompassDirection direction;      /* Compass direction           */
    QubitState       state;          /* Quantum-like state          */
    bool             superposed;     /* In superposition?           */
    int              entangled_with; /* Partner index (-1 if none)  */
} RingQubit;

/* Ring byte (8 qubits = 1 MMUKO byte) */
typedef struct {
    uint8_t          raw_value;
    RingQubit        qubits[RING_QUBITS];
    int              base_index;     /* 1–12 base in MMUKO system   */
    CompassDirection primary_super;  /* Primary superposition dir   */
    CompassDirection secondary_super;/* Secondary superposition dir */
} RingByte;

/* Vacuum medium (gravity reference frame) */
typedef struct {
    double gravity;     /* G_VACUUM = 9.8              */
    double lepton;      /* G_VACUUM / 10  = 0.98       */
    double muon;        /* G_LEPTON / 10  = 0.098      */
    double deep;        /* G_MUON   / 10  = 0.0098     */
} VacuumMedium;

/* Ring boot state machine */
typedef struct {
    RingBootPhase     phase;
    RingBootPhase     prev_phase;
    uint8_t           transition_count;
    uint8_t           nsigii_code;       /* NSIGII_YES/NO/MAYBE     */
    NSIGIIColor       color;             /* Three-way graph color    */
    uint16_t          flags;
} RingBootStateMachine;

/* Complete ring boot system */
typedef struct {
    RingByte*             memory_map;
    size_t                memory_size;
    VacuumMedium          medium;
    CompassDirection      frame_of_reference;
    RingBootStateMachine  state_machine;
    BootSectorContext*    boot_sector;
    bool                  boot_complete;
    uint32_t              boot_timestamp;
} RingBootSystem;

/* ================================================================
 * FUNCTION PROTOTYPES
 * ================================================================ */

/* System lifecycle */
RingBootSystem* ringboot_create(size_t memory_size);
void            ringboot_destroy(RingBootSystem *sys);

/* Boot sequence execution */
int             ringboot_execute(RingBootSystem *sys);
int             ringboot_phase_sparse(RingBootSystem *sys);
int             ringboot_phase_remember(RingBootSystem *sys);
int             ringboot_phase_active(RingBootSystem *sys);
int             ringboot_phase_verify(RingBootSystem *sys);

/* Qubit ring operations */
void            ringboot_init_qubit_ring(RingByte *byte);
QubitState      ringboot_resolve_state(int index, uint8_t byte_val);
QubitState      ringboot_flip_state(QubitState s);
uint8_t         ringboot_rotate_bits(uint8_t value, int n);

/* Vacuum medium */
VacuumMedium    ringboot_init_vacuum(void);

/* Superposition lookup */
void            ringboot_lookup_superposition(int base,
                    CompassDirection *primary,
                    CompassDirection *secondary);
int             ringboot_round_to_even_base(int base);

/* Compass and state utilities */
const char*     compass_direction_name(CompassDirection dir);
const char*     qubit_state_name(QubitState state);
const char*     ringboot_phase_name(RingBootPhase phase);
double          compass_spin_value(CompassDirection dir);

/* State machine transitions */
void            ringboot_transition(RingBootSystem *sys, RingBootPhase phase);

/* Diagnostics */
void            ringboot_print_qubit(const RingQubit *q);
void            ringboot_print_system(const RingBootSystem *sys);

#ifdef __cplusplus
}
#endif

#endif /* MMUKO_RINGBOOT_H */
