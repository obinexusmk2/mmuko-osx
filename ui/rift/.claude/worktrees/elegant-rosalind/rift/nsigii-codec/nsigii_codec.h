/*
 * NSIGII Polygatic Video Codec - C Library
 * Version: 7.0.0
 * Protocol: Human Rights Verification System
 * 
 * This is the C implementation of the NSIGII codec for RIFT pipeline stage 001.
 * Provides trident channel architecture with discriminant flash verification.
 */

#ifndef NSIGII_CODEC_H
#define NSIGII_CODEC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/* ============================================================================
 * VERSION AND CONSTANTS
 * ============================================================================ */
#define NSIGII_VERSION_MAJOR 7
#define NSIGII_VERSION_MINOR 0
#define NSIGII_VERSION_PATCH 0
#define NSIGII_VERSION_STRING "7.0.0"

#define NSIGII_MAGIC "NSIGII\0\0"
#define NSIGII_HEADER_SIZE 32
#define NSIGII_TRIDENT_CHANNELS 3

/* ============================================================================
 * TRIDENT CHANNEL IDENTIFIERS
 * ============================================================================ */
typedef enum {
    NSIGII_CHANNEL_TRANSMITTER = 0,  /* 127.0.0.1 - Encoder (ORDER) */
    NSIGII_CHANNEL_RECEIVER = 1,     /* 127.0.0.2 - Decoder (CHAOS) */
    NSIGII_CHANNEL_VERIFIER = 2      /* 127.0.0.3 - Verifier (CONSENSUS) */
} NsigiiChannelId;

/* ============================================================================
 * BIPOLAR STATES
 * ============================================================================ */
typedef enum {
    NSIGII_STATE_ORDER = 0x01,
    NSIGII_STATE_CHAOS = 0x00,
    NSIGII_STATE_VERIFIED = 0xFF
} NsigiiState;

/* ============================================================================
 * RWX PERMISSIONS
 * ============================================================================ */
typedef enum {
    NSIGII_RWX_READ = 0x04,
    NSIGII_RWX_WRITE = 0x02,
    NSIGII_RWX_EXECUTE = 0x01,
    NSIGII_RWX_FULL = 0x07
} NsigiiRWX;

/* ============================================================================
 * DISCRIMINANT FLASH STATES
 * ============================================================================ */
typedef enum {
    NSIGII_DISCRIMINANT_ORDER = 0,      /* Δ > 0: Two real roots */
    NSIGII_DISCRIMINANT_CONSENSUS = 1,  /* Δ = 0: One root (flash point) */
    NSIGII_DISCRIMINANT_CHAOS = 2       /* Δ < 0: Complex roots */
} NsigiiDiscriminantState;

/* ============================================================================
 * ENZYME OPERATIONS
 * ============================================================================ */
typedef enum {
    NSIGII_ENZYME_CREATE = 0,
    NSIGII_ENZYME_DESTROY,
    NSIGII_ENZYME_BUILD,
    NSIGII_ENZYME_BREAK,
    NSIGII_ENZYME_RENEW,
    NSIGII_ENZYME_REPAIR
} NsigiiEnzymeOperation;

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/* Flash Buffer: 1/2 + 1/2 = 1 */
typedef struct {
    uint8_t* half1;
    uint8_t* half2;
    size_t half_size;
} NsigiiFlashBuffer;

/* RB-AVL Tree Node */
typedef struct NsigiiRBNode {
    uint32_t key;
    uint8_t val;
    uint8_t polarity;
    double confidence;
    uint8_t color;        /* Red-Black color */
    uint8_t height;       /* AVL height */
    struct NsigiiRBNode* left;
    struct NsigiiRBNode* right;
    struct NsigiiRBNode* parent;
} NsigiiRBNode;

/* RB-AVL Tree */
typedef struct {
    NsigiiRBNode* root;
    int streak[256];
} NsigiiRBTree;

/* Filter Flash: Discriminant verification */
typedef struct {
    double a;
    double b;
    double c;
    NsigiiDiscriminantState state;
} NsigiiFilterFlash;

/* Trident Header */
typedef struct {
    uint8_t channel_id;
    uint32_t sequence_token;
    uint64_t timestamp;
    uint8_t codec_version;
} NsigiiTridentHeader;

/* Trident Payload */
typedef struct {
    uint8_t message_hash[32];  /* SHA-256 */
    uint32_t content_length;
    uint8_t* content;
} NsigiiTridentPayload;

/* Trident Verification */
typedef struct {
    uint8_t rwx_flags;
    uint8_t consensus_sig[64];
    char human_rights_tag[32];
} NsigiiTridentVerification;

/* Trident Topology */
typedef struct {
    uint8_t next_channel;
    uint8_t prev_channel;
    uint8_t wheel_position;  /* 0°, 120°, 240°, 360° */
} NsigiiTridentTopology;

/* Trident Packet */
typedef struct {
    NsigiiTridentHeader header;
    NsigiiTridentPayload payload;
    NsigiiTridentVerification verification;
    NsigiiTridentTopology topology;
} NsigiiTridentPacket;

/* Trident Channel */
typedef struct {
    uint8_t id;
    char loopback_addr[16];
    double codec_ratio;
    uint8_t state;
    NsigiiRBTree* tree;
    NsigiiFilterFlash* flash;
} NsigiiTridentChannel;

/* NSIGII Codec Context */
typedef struct {
    int width;
    int height;
    NsigiiTridentChannel* channels[3];
    NsigiiRBTree* tree;
    NsigiiFlashBuffer* flash_buffer;
    
    /* Statistics */
    uint32_t frames_encoded;
    uint32_t frames_decoded;
    uint32_t chaos_count;
    uint32_t order_count;
    uint32_t consensus_count;
} NsigiiCodec;

/* NSIGII File Header */
typedef struct {
    char magic[8];
    char version[8];
    uint32_t width;
    uint32_t height;
    uint32_t frame_count;
    uint32_t reserved;
} NsigiiFileHeader;

/* ============================================================================
 * API FUNCTIONS
 * ============================================================================ */

/* Codec Lifecycle */
NsigiiCodec* nsigii_codec_create(int width, int height);
void nsigii_codec_destroy(NsigiiCodec* codec);

/* Flash Buffer */
NsigiiFlashBuffer* nsigii_flash_buffer_create(size_t size);
void nsigii_flash_buffer_destroy(NsigiiFlashBuffer* buffer);
uint8_t* nsigii_flash_unite(NsigiiFlashBuffer* buffer);      /* 1/2 + 1/2 = 1 */
uint8_t* nsigii_flash_multiply(NsigiiFlashBuffer* buffer);   /* 1/2 × 1/2 = 1/4 */

/* RB-AVL Tree */
NsigiiRBTree* nsigii_rbtree_create(void);
void nsigii_rbtree_destroy(NsigiiRBTree* tree);
void nsigii_rbtree_insert(NsigiiRBTree* tree, uint32_t key, uint8_t val, double confidence, uint8_t polarity);
NsigiiRBNode* nsigii_rbtree_find(NsigiiRBTree* tree, uint32_t key);
void nsigii_rbtree_mark_measurement(NsigiiRBTree* tree, uint32_t key, double confidence, uint8_t polarity);

/* Filter Flash */
NsigiiFilterFlash* nsigii_filter_flash_create(double a, double b, double c);
void nsigii_filter_flash_destroy(NsigiiFilterFlash* flash);
double nsigii_filter_flash_compute_discriminant(NsigiiFilterFlash* flash);
NsigiiDiscriminantState nsigii_filter_flash_context_switch(NsigiiFilterFlash* flash);
bool nsigii_filter_flash_quadratic_roots(NsigiiFilterFlash* flash, double* root1, double* root2);

/* ROPEN Sparse Duplex Encoding */
uint8_t nsigii_conjugate(uint8_t x);  /* 0xF ⊕ x */
uint8_t* nsigii_rift_encode(const uint8_t* input, size_t len, bool polarity_a, NsigiiRBTree* tree, size_t* out_len);

/* Trident Channel Operations */
NsigiiTridentChannel* nsigii_trident_channel_create(uint8_t id);
void nsigii_trident_channel_destroy(NsigiiTridentChannel* channel);
NsigiiTridentPacket nsigii_channel_encode_message(NsigiiTridentChannel* channel, const uint8_t* content, size_t len);
NsigiiTridentPacket nsigii_channel_decode_packet(NsigiiTridentChannel* channel, NsigiiTridentPacket packet);
bool nsigii_channel_verify_packet(NsigiiTridentChannel* channel, NsigiiTridentPacket* packet);

/* Frame Encoding/Decoding */
int nsigii_encode_frame(NsigiiCodec* codec, const uint8_t* rgb_frame, uint8_t** output, size_t* output_len);
int nsigii_decode_frame(NsigiiCodec* codec, const uint8_t* input, size_t input_len, uint8_t** rgb_frame);

/* Color Space Conversion */
uint8_t* nsigii_rgb_to_yuv420(const uint8_t* rgb, int width, int height);
uint8_t* nsigii_yuv420_to_rgb(const uint8_t* yuv, int width, int height);

/* File I/O */
int nsigii_write_header(FILE* fp, const NsigiiFileHeader* header);
int nsigii_read_header(FILE* fp, NsigiiFileHeader* header);
int nsigii_write_frame(FILE* fp, const uint8_t* frame_data, size_t frame_len);

/* Utility */
const char* nsigii_version_string(void);
const char* nsigii_discriminant_state_to_string(NsigiiDiscriminantState state);
double nsigii_bipartite_consensus(const uint8_t* content, size_t len, uint8_t wheel_pos);

#ifdef __cplusplus
}
#endif

#endif /* NSIGII_CODEC_H */
