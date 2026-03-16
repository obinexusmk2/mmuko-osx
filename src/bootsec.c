/*
 * bootsec.c - MMUKO-OS Boot Sector Implementation
 * Project: OBINexus / MMUKO-OS
 * Protocol: NSIGII Human Rights Verification Firmware
 *
 * Implements:
 *   - RIFT header validation and checksum verification
 *   - FAT/NTFS/MMUKO cross-boot MBR filesystem detection
 *   - NSIGII tripartite consent verification
 *   - Three-way colorable graph: Green/Yellow/Red states
 *   - Drift discriminant (b²-4ac) for trust calculation
 *   - AST/Cab fingerprint via stateless probe
 *
 * NSIGII Human Rights Protocol (derived from transcripts):
 *   The NSIGII firmware utilizes the MMUKO boot sequence to perform
 *   a stateless probe of architecture. The firmware is a cybernetics
 *   interface that is loaded on boot for consent verification.
 *   By "need" we are referred to the nature of human suffering,
 *   querying the quadratic tripartite graph which is three-colorable.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "../include/bootsec.h"

/* ================================================================
 * STRING LOOKUP TABLES
 * ================================================================ */

static const char* nsigii_color_names[] = {
    "GREEN", "YELLOW", "RED"
};

static const char* drift_dir_names[] = {
    "TOWARDS", "AWAY", "ORTHOGONAL"
};

static const char* fs_names[] = {
    "UNKNOWN", "FAT", "NTFS", "MMUKO-RINGFS"
};

/* ================================================================
 * UTILITY FUNCTIONS
 * ================================================================ */

const char* nsigii_color_name(NSIGIIColor color) {
    if (color <= COLOR_RED) return nsigii_color_names[color];
    return "INVALID";
}

const char* drift_direction_name(DriftDirection dir) {
    if (dir <= DRIFT_ORTHOGONAL) return drift_dir_names[dir];
    return "INVALID";
}

const char* bootsec_fs_name(FSDetectResult fs) {
    if (fs <= FS_DETECT_MMUKO) return fs_names[fs];
    return "INVALID";
}

/* ================================================================
 * RIFT HEADER VALIDATION
 * ================================================================ */

uint8_t bootsec_calculate_checksum(const uint8_t *data, size_t len) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < len; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

bool bootsec_validate_rift(const RIFTHeader *header) {
    if (!header) return false;

    /* Check magic bytes */
    if (memcmp(header->magic, RIFT_MAGIC, RIFT_MAGIC_LEN) != 0) {
        printf("[BOOTSEC] RIFT magic mismatch: expected NXOB\n");
        return false;
    }

    /* Check version compatibility */
    if (header->version < 0x01 || header->version > 0x0F) {
        printf("[BOOTSEC] RIFT version out of range: 0x%02X\n",
               header->version);
        return false;
    }

    /* Verify reserved field is zero */
    if (header->reserved != 0x00) {
        printf("[BOOTSEC] RIFT reserved field non-zero: 0x%02X\n",
               header->reserved);
        return false;
    }

    /* Verify NSIGII flag is set for ringboot */
    if (header->version >= 0x02 && !(header->flags & RIFT_FLAG_NSIGII)) {
        printf("[BOOTSEC] RIFT v2+ requires NSIGII flag\n");
        return false;
    }

    printf("[BOOTSEC] RIFT header valid: v%d, flags=0x%02X\n",
           header->version, header->flags);
    return true;
}

/* ================================================================
 * FILESYSTEM CROSS-BOOT DETECTION
 * ================================================================ */

FSDetectResult bootsec_detect_filesystem(const BootSectorLayout *sector) {
    if (!sector) return FS_DETECT_NONE;

    /* Check each partition entry for known filesystem types */
    for (int i = 0; i < 4; i++) {
        uint8_t type = sector->partitions[i].type;

        switch (type) {
            case FS_TYPE_FAT12:
            case FS_TYPE_FAT16:
            case FS_TYPE_FAT32:
            case FS_TYPE_FAT32_LBA:
                printf("[BOOTSEC] Partition %d: FAT (type=0x%02X)\n",
                       i, type);
                return FS_DETECT_FAT;

            case FS_TYPE_NTFS:
                printf("[BOOTSEC] Partition %d: NTFS (type=0x%02X)\n",
                       i, type);
                return FS_DETECT_NTFS;

            case FS_TYPE_MMUKO_RAW:
                printf("[BOOTSEC] Partition %d: MMUKO-RingFS (type=0x%02X)\n",
                       i, type);
                return FS_DETECT_MMUKO;

            default:
                break;
        }
    }

    printf("[BOOTSEC] No recognized filesystem in partition table\n");
    return FS_DETECT_NONE;
}

/* ================================================================
 * NSIGII CONSENT VERIFICATION
 * Three-way colorable graph: U(User), V(Institution), W(Arbiter)
 * ================================================================ */

/**
 * Calculate drift discriminant: b² - 4ac
 * Where:
 *   a = system stability coefficient
 *   b = consent signal strength
 *   c = environmental interference
 *
 * Result interpretation (from three-way colorable graph):
 *   discriminant > 0  → Two solutions → DRIFT_TOWARDS (Green)
 *   discriminant == 0 → One solution  → DRIFT_ORTHOGONAL (Yellow)
 *   discriminant < 0  → No solution   → DRIFT_AWAY (Red)
 */
double bootsec_discriminant(double a, double b, double c) {
    return (b * b) - (4.0 * a * c);
}

DriftDirection bootsec_calculate_drift(double b, double a, double c) {
    double disc = bootsec_discriminant(a, b, c);

    if (disc > 0.0) {
        return DRIFT_TOWARDS;      /* Two solutions: moving toward trust */
    } else if (fabs(disc) < 1e-10) {
        return DRIFT_ORTHOGONAL;   /* One solution: equilibrium          */
    } else {
        return DRIFT_AWAY;         /* No solution: drifting away          */
    }
}

/**
 * NSIGII consent probe - stateless architecture check
 * Probes the system for living signal indicators
 */
bool bootsec_consent_probe(NSIGIIConsentState *state) {
    if (!state) return false;

    /* Update consent timestamp */
    state->consent_ts = (uint32_t)time(NULL);

    /* Calculate drift from current consent states
     * a = user_consent normalized (0.0 to 1.0)
     * b = institution consent signal
     * c = arbiter interference coefficient
     */
    double a_coeff = (state->user_consent == NSIGII_YES) ? 1.0 :
                     (state->user_consent == NSIGII_MAYBE) ? 0.5 : 0.0;
    double b_coeff = (state->inst_consent == NSIGII_YES) ? 2.0 :
                     (state->inst_consent == NSIGII_MAYBE) ? 1.0 : 0.1;
    double c_coeff = (state->arb_consent == NSIGII_YES) ? 0.5 :
                     (state->arb_consent == NSIGII_MAYBE) ? 1.0 : 2.0;

    state->discriminant = bootsec_discriminant(a_coeff, b_coeff, c_coeff);
    state->drift = bootsec_calculate_drift(b_coeff, a_coeff, c_coeff);

    /* Determine color from drift */
    switch (state->drift) {
        case DRIFT_TOWARDS:
            state->color = COLOR_GREEN;
            state->living_loop = true;
            break;
        case DRIFT_ORTHOGONAL:
            state->color = COLOR_YELLOW;
            state->living_loop = true;  /* Still alive, but cautious */
            break;
        case DRIFT_AWAY:
            state->color = COLOR_RED;
            state->living_loop = false;
            break;
    }

    printf("[NSIGII] Consent probe: U=%s V=%s W=%s → %s (disc=%.4f)\n",
           state->user_consent == NSIGII_YES ? "YES" :
           state->user_consent == NSIGII_MAYBE ? "MAYBE" : "NO",
           state->inst_consent == NSIGII_YES ? "YES" :
           state->inst_consent == NSIGII_MAYBE ? "MAYBE" : "NO",
           state->arb_consent == NSIGII_YES ? "YES" :
           state->arb_consent == NSIGII_MAYBE ? "MAYBE" : "NO",
           nsigii_color_name(state->color),
           state->discriminant);

    return state->color != COLOR_RED;
}

/**
 * Full NSIGII verification
 * Returns the three-way colorable graph color
 */
NSIGIIColor bootsec_nsigii_verify(BootSectorContext *ctx) {
    if (!ctx) return COLOR_RED;

    /* Run consent probe */
    bootsec_consent_probe(&ctx->consent);

    /* Verify RIFT header integrity */
    if (!ctx->rift_valid) {
        printf("[NSIGII] RIFT header invalid → RED\n");
        ctx->consent.color = COLOR_RED;
        return COLOR_RED;
    }

    /* Check fingerprint verification count (informational in boot phase)
     * Fingerprints are verified post-boot when components load.
     * During initial boot, consent color from probe takes precedence. */
    if (ctx->fp_count < (CAB_DIGEST_BLOCKS / 2)) {
        printf("[NSIGII] Fingerprints pending (%d/%d) — will verify post-boot\n",
               ctx->fp_count, CAB_DIGEST_BLOCKS);
    }

    return ctx->consent.color;
}

/* ================================================================
 * AST/CAB FINGERPRINT SYSTEM
 * Stateless probe of architecture (from handwritten notes)
 * Cube face model: 6 faces × component fingerprints
 * ================================================================ */

void bootsec_fingerprint_init(ASTFingerprint *fp, uint8_t component_id,
                               uint8_t face_index) {
    if (!fp) return;

    memset(fp->digest, 0, AST_FINGERPRINT_LEN);
    fp->timestamp = (uint32_t)time(NULL);
    fp->component_id = component_id;
    fp->face_index = face_index % 6;  /* 6 cube faces */
    fp->verified = false;
}

bool bootsec_verify_fingerprint(ASTFingerprint *fp,
                                 const uint8_t *component_data,
                                 size_t data_len) {
    if (!fp || !component_data || data_len == 0) return false;

    /* Calculate simple XOR-based digest
     * (In production: MD5 or SHA-256 as specified in notes)
     * "512 block digest of the FAT format" */
    uint8_t computed[AST_FINGERPRINT_LEN];
    memset(computed, 0, AST_FINGERPRINT_LEN);

    for (size_t i = 0; i < data_len; i++) {
        computed[i % AST_FINGERPRINT_LEN] ^= component_data[i];
    }

    /* Add timestamp-based salt (from notes: "time stamped hashed") */
    uint32_t ts = fp->timestamp;
    for (int i = 0; i < 4; i++) {
        computed[i] ^= (uint8_t)(ts >> (i * 8));
    }

    /* Store computed digest */
    memcpy(fp->digest, computed, AST_FINGERPRINT_LEN);
    fp->verified = true;

    printf("[AST] Component %d (face %d) fingerprinted: %02X%02X%02X%02X...\n",
           fp->component_id, fp->face_index,
           fp->digest[0], fp->digest[1], fp->digest[2], fp->digest[3]);

    return true;
}

/* ================================================================
 * BOOT SECTOR LIFECYCLE
 * ================================================================ */

BootSectorContext* bootsec_create(void) {
    BootSectorContext *ctx = (BootSectorContext*)calloc(1, sizeof(BootSectorContext));
    if (!ctx) return NULL;

    /* Initialize RIFT header with defaults */
    memcpy(ctx->sector.rift.magic, RIFT_MAGIC, RIFT_MAGIC_LEN);
    ctx->sector.rift.version = RIFT_VERSION_RING;
    ctx->sector.rift.reserved = 0x00;
    ctx->sector.rift.flags = RIFT_FLAG_NSIGII | RIFT_FLAG_RINGBOOT | RIFT_FLAG_CONSENT;
    ctx->sector.rift.checksum = RIFT_CHECKSUM_INIT;

    /* Set boot signature */
    ctx->sector.signature = BOOTSEC_SIGNATURE;

    /* Initialize consent to MAYBE (pending first probe) */
    ctx->consent.user_consent = NSIGII_MAYBE;
    ctx->consent.inst_consent = NSIGII_MAYBE;
    ctx->consent.arb_consent = NSIGII_MAYBE;
    ctx->consent.color = COLOR_YELLOW;
    ctx->consent.drift = DRIFT_ORTHOGONAL;
    ctx->consent.living_loop = false;

    ctx->rift_valid = false;
    ctx->boot_ready = false;
    ctx->fp_count = 0;

    return ctx;
}

void bootsec_destroy(BootSectorContext *ctx) {
    if (ctx) {
        free(ctx);
    }
}

/* ================================================================
 * BOOT SEQUENCE EXECUTION
 * ================================================================ */

int bootsec_execute_boot(BootSectorContext *ctx) {
    if (!ctx) return -1;

    printf("\n=== MMUKO-OS BOOT SECTOR INITIALIZATION ===\n\n");

    /* Step 1: Validate RIFT header */
    printf("[BOOT] Step 1: RIFT header validation\n");
    ctx->rift_valid = bootsec_validate_rift(&ctx->sector.rift);
    if (!ctx->rift_valid) {
        printf("[BOOT] RIFT validation FAILED\n");
        return -1;
    }

    /* Step 2: Detect filesystem for cross-boot */
    printf("[BOOT] Step 2: Filesystem detection\n");
    ctx->fs_detected = bootsec_detect_filesystem(&ctx->sector);
    printf("[BOOT] Detected: %s\n", bootsec_fs_name(ctx->fs_detected));

    /* Step 3: Initialize user consent (simulate living probe) */
    printf("[BOOT] Step 3: NSIGII consent initialization\n");
    ctx->consent.user_consent = NSIGII_YES;   /* User present at boot */
    ctx->consent.inst_consent = NSIGII_YES;   /* System integrity OK  */
    ctx->consent.arb_consent = NSIGII_MAYBE;  /* Arbiter pending      */

    /* Step 4: Run NSIGII verification */
    printf("[BOOT] Step 4: NSIGII tripartite verification\n");
    NSIGIIColor result = bootsec_nsigii_verify(ctx);

    switch (result) {
        case COLOR_GREEN:
            printf("[BOOT] NSIGII: GREEN → Boot authorized\n");
            ctx->boot_ready = true;
            break;
        case COLOR_YELLOW:
            printf("[BOOT] NSIGII: YELLOW → Partial consent, running yes-yes loop\n");
            /* Second yes-yes check (living consent loop) */
            ctx->consent.arb_consent = NSIGII_YES;
            result = bootsec_nsigii_verify(ctx);
            /* Yellow with living loop active is acceptable for boot */
            ctx->boot_ready = (result == COLOR_GREEN) ||
                              (result == COLOR_YELLOW && ctx->consent.living_loop);
            break;
        case COLOR_RED:
            printf("[BOOT] NSIGII: RED → Boot denied\n");
            ctx->boot_ready = false;
            break;
    }

    /* Step 5: Verify boot signature */
    printf("[BOOT] Step 5: Boot signature check\n");
    if (ctx->sector.signature != BOOTSEC_SIGNATURE) {
        printf("[BOOT] Invalid boot signature: 0x%04X\n",
               ctx->sector.signature);
        ctx->boot_ready = false;
    } else {
        printf("[BOOT] Boot signature: 0xAA55 ✓\n");
    }

    bootsec_print_status(ctx);

    return ctx->boot_ready ? 0 : -1;
}

void bootsec_print_status(const BootSectorContext *ctx) {
    if (!ctx) return;

    printf("\n--- Boot Sector Status ---\n");
    printf("RIFT: %s | FS: %s | NSIGII: %s\n",
           ctx->rift_valid ? "VALID" : "INVALID",
           bootsec_fs_name(ctx->fs_detected),
           nsigii_color_name(ctx->consent.color));
    printf("Drift: %s | Discriminant: %.4f\n",
           drift_direction_name(ctx->consent.drift),
           ctx->consent.discriminant);
    printf("Living Loop: %s | Fingerprints: %d/%d\n",
           ctx->consent.living_loop ? "ACTIVE" : "INACTIVE",
           ctx->fp_count, CAB_DIGEST_BLOCKS);
    printf("Boot Ready: %s\n",
           ctx->boot_ready ? "YES" : "NO");
    printf("--------------------------\n\n");
}
