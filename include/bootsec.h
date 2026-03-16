/*
 * bootsec.h - MMUKO-OS Boot Sector Interface
 * Project: OBINexus / MMUKO-OS
 * Protocol: NSIGII Human Rights Verification Firmware
 *
 * Provides C-level abstraction for:
 *   - RIFT header validation
 *   - Boot sector layout (512-byte model)
 *   - FAT/NTFS cross-boot MBR detection
 *   - AST/Cab fingerprint verification
 *   - Three-way colorable graph consent states
 *
 * NSIGII Human Rights Integration:
 *   The boot sector enforces living consent at the firmware level.
 *   Every boot action requires tripartite verification:
 *     U (User) → V (Institution) → W (Arbiter/Third-party)
 *   Drift discriminant (b²-4ac) determines system trust state.
 */

#ifndef MMUKO_BOOTSEC_H
#define MMUKO_BOOTSEC_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * RIFT MAGIC & VERSION CONSTANTS
 * ================================================================ */

#define RIFT_MAGIC          "NXOB"
#define RIFT_MAGIC_LEN      4
#define RIFT_VERSION_RING   0x02
#define RIFT_CHECKSUM_INIT  0xFE
#define RIFT_FLAG_NSIGII    0x01
#define RIFT_FLAG_RINGBOOT  0x02
#define RIFT_FLAG_CONSENT   0x04

/* Boot sector geometry */
#define BOOTSEC_SIZE        512
#define BOOTSEC_HEADER_SIZE 8
#define BOOTSEC_CODE_SIZE   502
#define BOOTSEC_SIG_OFFSET  510
#define BOOTSEC_SIGNATURE   0xAA55

/* NSIGII Trinary Protocol States */
#define NSIGII_YES          0x55    /* 01010101 - Green: Verified     */
#define NSIGII_NO           0xAA    /* 10101010 - Red:   Denied       */
#define NSIGII_MAYBE        0x00    /* 00000000 - Yellow: Pending     */

/* Filesystem type flags (cross-boot MBR) */
#define FS_TYPE_UNKNOWN     0x00
#define FS_TYPE_FAT12       0x01
#define FS_TYPE_FAT16       0x04
#define FS_TYPE_FAT32       0x0B
#define FS_TYPE_FAT32_LBA   0x0C
#define FS_TYPE_NTFS        0x07
#define FS_TYPE_LINUX_EXT   0x83
#define FS_TYPE_MMUKO_RAW   0xEF    /* MMUKO native ring-zone FS */

/* AST/Cab fingerprint constants */
#define AST_FINGERPRINT_LEN     32
#define CAB_DIGEST_BLOCKS       16
#define PROBE_LATTICE_HALF      0x08

/* Three-way colorable graph colors */
typedef enum {
    COLOR_GREEN  = 0,   /* System OK - consent verified        */
    COLOR_YELLOW = 1,   /* Warning - partial consent / drift   */
    COLOR_RED    = 2    /* Danger - consent denied / violation  */
} NSIGIIColor;

/* Drift direction (tripartite graph model) */
typedef enum {
    DRIFT_TOWARDS    = 0,   /* Moving toward resolution        */
    DRIFT_AWAY       = 1,   /* Moving away from resolution     */
    DRIFT_ORTHOGONAL = 2    /* Perpendicular / indeterminate   */
} DriftDirection;

/* Filesystem detection result */
typedef enum {
    FS_DETECT_NONE    = 0,
    FS_DETECT_FAT     = 1,
    FS_DETECT_NTFS    = 2,
    FS_DETECT_MMUKO   = 3
} FSDetectResult;

/* ================================================================
 * STRUCTURES
 * ================================================================ */

/* RIFT Header (8 bytes, packed for boot sector alignment) */
typedef struct __attribute__((packed)) {
    uint8_t  magic[4];       /* "NXOB" - OBINEXUS signature     */
    uint8_t  version;        /* Protocol version                 */
    uint8_t  reserved;       /* Must be 0x00                     */
    uint8_t  checksum;       /* XOR checksum of header           */
    uint8_t  flags;          /* NSIGII | RINGBOOT | CONSENT      */
} RIFTHeader;

/* MBR Partition Table Entry (16 bytes) */
typedef struct __attribute__((packed)) {
    uint8_t  status;         /* 0x80 = active, 0x00 = inactive   */
    uint8_t  chs_start[3];  /* CHS address of first sector      */
    uint8_t  type;           /* Filesystem type code             */
    uint8_t  chs_end[3];    /* CHS address of last sector       */
    uint32_t lba_start;     /* LBA of first sector              */
    uint32_t sector_count;  /* Number of sectors                */
} MBRPartitionEntry;

/* Boot Sector Layout (512 bytes) */
typedef struct __attribute__((packed)) {
    RIFTHeader        rift;              /* 8 bytes: RIFT header        */
    uint8_t           boot_code[438];    /* Boot code                   */
    MBRPartitionEntry partitions[4];     /* 64 bytes: 4 partition table */
    uint16_t          signature;         /* 0xAA55                      */
} BootSectorLayout;

/* AST/Cab Fingerprint (stateless probe artifact) */
typedef struct {
    uint8_t  digest[AST_FINGERPRINT_LEN]; /* MD5/SHA256 fingerprint   */
    uint32_t timestamp;                    /* Boot-time timestamp      */
    uint8_t  component_id;                 /* Component being verified */
    uint8_t  face_index;                   /* Cube face (0-5)          */
    bool     verified;                     /* Probe result             */
} ASTFingerprint;

/* NSIGII Consent State (tripartite) */
typedef struct {
    NSIGIIColor  color;          /* Current graph color             */
    DriftDirection drift;        /* Drift direction                 */
    double       discriminant;   /* b²-4ac value                    */
    uint8_t      user_consent;   /* U: NSIGII_YES/NO/MAYBE         */
    uint8_t      inst_consent;   /* V: Institution consent          */
    uint8_t      arb_consent;    /* W: Arbiter consent              */
    uint32_t     consent_ts;     /* Last consent timestamp          */
    bool         living_loop;    /* yes-yes loop active?            */
} NSIGIIConsentState;

/* Boot Sector Context (runtime state) */
typedef struct {
    BootSectorLayout  sector;        /* Raw 512-byte sector         */
    FSDetectResult    fs_detected;   /* Detected filesystem         */
    NSIGIIConsentState consent;      /* NSIGII consent state        */
    ASTFingerprint    fingerprints[CAB_DIGEST_BLOCKS]; /* Component FPs */
    uint8_t           fp_count;      /* Number of verified FPs      */
    bool              rift_valid;    /* RIFT header validated?       */
    bool              boot_ready;    /* All phases passed?           */
} BootSectorContext;

/* ================================================================
 * FUNCTION PROTOTYPES
 * ================================================================ */

/* Boot sector initialization and validation */
BootSectorContext* bootsec_create(void);
void              bootsec_destroy(BootSectorContext *ctx);
bool              bootsec_validate_rift(const RIFTHeader *header);
uint8_t           bootsec_calculate_checksum(const uint8_t *data, size_t len);

/* Filesystem cross-boot detection */
FSDetectResult    bootsec_detect_filesystem(const BootSectorLayout *sector);
const char*       bootsec_fs_name(FSDetectResult fs);

/* NSIGII consent verification */
NSIGIIColor       bootsec_nsigii_verify(BootSectorContext *ctx);
bool              bootsec_consent_probe(NSIGIIConsentState *state);
DriftDirection    bootsec_calculate_drift(double b, double a, double c);
double            bootsec_discriminant(double a, double b, double c);

/* AST/Cab fingerprint system */
bool              bootsec_verify_fingerprint(ASTFingerprint *fp,
                                              const uint8_t *component_data,
                                              size_t data_len);
void              bootsec_fingerprint_init(ASTFingerprint *fp,
                                            uint8_t component_id,
                                            uint8_t face_index);

/* Boot sequence execution */
int               bootsec_execute_boot(BootSectorContext *ctx);
void              bootsec_print_status(const BootSectorContext *ctx);

/* Utility */
const char*       nsigii_color_name(NSIGIIColor color);
const char*       drift_direction_name(DriftDirection dir);

#ifdef __cplusplus
}
#endif

#endif /* MMUKO_BOOTSEC_H */
