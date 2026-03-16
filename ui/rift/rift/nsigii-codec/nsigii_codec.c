/*
 * NSIGII Polygatic Video Codec - C Implementation
 * Version: 7.0.0
 * 
 * Implements trident channel architecture with:
 * - ROPEN sparse duplex encoding (2→1)
 * - RB-AVL tree with confidence-based pruning
 * - Discriminant flash verification
 * - Bipolar enzyme operations
 */

#include "nsigii_codec.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */
#define PRUNE_THRESHOLD 0.5
#define EPSILON_PAD 0x00
#define POLARITY_POS '+'
#define POLARITY_NEG '-'

/* ============================================================================
 * FLASH BUFFER IMPLEMENTATION
 * ============================================================================ */
NsigiiFlashBuffer* nsigii_flash_buffer_create(size_t size) {
    NsigiiFlashBuffer* buffer = (NsigiiFlashBuffer*)malloc(sizeof(NsigiiFlashBuffer));
    if (!buffer) return NULL;
    
    buffer->half_size = size / 2;
    buffer->half1 = (uint8_t*)calloc(buffer->half_size, sizeof(uint8_t));
    buffer->half2 = (uint8_t*)calloc(buffer->half_size, sizeof(uint8_t));
    
    if (!buffer->half1 || !buffer->half2) {
        free(buffer->half1);
        free(buffer->half2);
        free(buffer);
        return NULL;
    }
    
    return buffer;
}

void nsigii_flash_buffer_destroy(NsigiiFlashBuffer* buffer) {
    if (!buffer) return;
    free(buffer->half1);
    free(buffer->half2);
    free(buffer);
}

uint8_t* nsigii_flash_unite(NsigiiFlashBuffer* buffer) {
    if (!buffer) return NULL;
    
    uint8_t* result = (uint8_t*)malloc(buffer->half_size * 2);
    if (!result) return NULL;
    
    memcpy(result, buffer->half1, buffer->half_size);
    memcpy(result + buffer->half_size, buffer->half2, buffer->half_size);
    
    return result;
}

uint8_t* nsigii_flash_multiply(NsigiiFlashBuffer* buffer) {
    if (!buffer) return NULL;
    
    size_t quarter_size = buffer->half_size / 2;
    uint8_t* result = (uint8_t*)malloc(quarter_size);
    if (!result) return NULL;
    
    /* XOR the halves to create dimensional reduction: 1/2 × 1/2 = 1/4 */
    for (size_t i = 0; i < quarter_size; i++) {
        if (i < buffer->half_size) {
            result[i] = buffer->half1[i] ^ buffer->half2[i];
        }
    }
    
    return result;
}

/* ============================================================================
 * RB-AVL TREE IMPLEMENTATION
 * ============================================================================ */
static uint8_t rbtree_height(NsigiiRBNode* node) {
    return node ? node->height : 0;
}

static int rbtree_balance_factor(NsigiiRBNode* node) {
    if (!node) return 0;
    return (int)rbtree_height(node->left) - (int)rbtree_height(node->right);
}

static void rbtree_update_height(NsigiiRBNode* node) {
    if (!node) return;
    uint8_t left_h = rbtree_height(node->left);
    uint8_t right_h = rbtree_height(node->right);
    node->height = (left_h > right_h ? left_h : right_h) + 1;
}

static NsigiiRBNode* rbtree_rotate_left(NsigiiRBNode* x) {
    if (!x || !x->right) return x;
    
    NsigiiRBNode* y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    
    y->parent = x->parent;
    y->left = x;
    x->parent = y;
    
    rbtree_update_height(x);
    rbtree_update_height(y);
    
    return y;
}

static NsigiiRBNode* rbtree_rotate_right(NsigiiRBNode* x) {
    if (!x || !x->left) return x;
    
    NsigiiRBNode* y = x->left;
    x->left = y->right;
    if (y->right) y->right->parent = x;
    
    y->parent = x->parent;
    y->right = x;
    x->parent = y;
    
    rbtree_update_height(x);
    rbtree_update_height(y);
    
    return y;
}

static NsigiiRBNode* rbtree_rebalance(NsigiiRBNode* node) {
    if (!node) return NULL;
    
    rbtree_update_height(node);
    int bf = rbtree_balance_factor(node);
    
    if (bf > 1) {  /* Left heavy */
        if (rbtree_balance_factor(node->left) < 0) {
            node->left = rbtree_rotate_left(node->left);
        }
        return rbtree_rotate_right(node);
    } else if (bf < -1) {  /* Right heavy */
        if (rbtree_balance_factor(node->right) > 0) {
            node->right = rbtree_rotate_right(node->right);
        }
        return rbtree_rotate_left(node);
    }
    
    return node;
}

static NsigiiRBNode* rbtree_insert_recursive(NsigiiRBNode* root, NsigiiRBNode* node) {
    if (!root) return node;
    
    if (node->key < root->key) {
        root->left = rbtree_insert_recursive(root->left, node);
        if (root->left) root->left->parent = root;
    } else if (node->key > root->key) {
        root->right = rbtree_insert_recursive(root->right, node);
        if (root->right) root->right->parent = root;
    } else {
        /* Update in place */
        root->val = node->val;
        root->confidence = node->confidence;
        root->polarity = node->polarity;
        free(node);
        return root;
    }
    
    return rbtree_rebalance(root);
}

NsigiiRBTree* nsigii_rbtree_create(void) {
    NsigiiRBTree* tree = (NsigiiRBTree*)calloc(1, sizeof(NsigiiRBTree));
    return tree;
}

static void rbtree_destroy_recursive(NsigiiRBNode* node) {
    if (!node) return;
    rbtree_destroy_recursive(node->left);
    rbtree_destroy_recursive(node->right);
    free(node);
}

void nsigii_rbtree_destroy(NsigiiRBTree* tree) {
    if (!tree) return;
    rbtree_destroy_recursive(tree->root);
    free(tree);
}

void nsigii_rbtree_insert(NsigiiRBTree* tree, uint32_t key, uint8_t val, double confidence, uint8_t polarity) {
    if (!tree) return;
    
    NsigiiRBNode* node = (NsigiiRBNode*)calloc(1, sizeof(NsigiiRBNode));
    if (!node) return;
    
    node->key = key;
    node->val = val;
    node->confidence = confidence;
    node->polarity = polarity;
    node->color = 1;  /* Red */
    node->height = 1;
    
    if (!tree->root) {
        node->color = 0;  /* Root is black */
        tree->root = node;
        return;
    }
    
    tree->root = rbtree_insert_recursive(tree->root, node);
}

NsigiiRBNode* nsigii_rbtree_find(NsigiiRBTree* tree, uint32_t key) {
    if (!tree) return NULL;
    
    NsigiiRBNode* cur = tree->root;
    while (cur) {
        if (key == cur->key) return cur;
        if (key < cur->key) cur = cur->left;
        else cur = cur->right;
    }
    return NULL;
}

void nsigii_rbtree_mark_measurement(NsigiiRBTree* tree, uint32_t key, double confidence, uint8_t polarity) {
    if (!tree) return;
    
    NsigiiRBNode* node = nsigii_rbtree_find(tree, key);
    if (!node) return;
    
    node->confidence = confidence;
    if (polarity != 0) node->polarity = polarity;
    
    /* Pruning decision based on confidence and polarity */
    if (confidence < PRUNE_THRESHOLD || node->polarity == POLARITY_NEG) {
        int idx = key & 0xFF;
        tree->streak[idx]++;
        if (tree->streak[idx] >= 1) {
            node->val = 0;
            node->confidence = 0.0;
        }
    } else {
        tree->streak[key & 0xFF] = 0;
    }
}

/* ============================================================================
 * FILTER FLASH IMPLEMENTATION
 * ============================================================================ */
NsigiiFilterFlash* nsigii_filter_flash_create(double a, double b, double c) {
    NsigiiFilterFlash* flash = (NsigiiFilterFlash*)malloc(sizeof(NsigiiFilterFlash));
    if (!flash) return NULL;
    
    flash->a = a;
    flash->b = b;
    flash->c = c;
    flash->state = NSIGII_DISCRIMINANT_ORDER;
    
    return flash;
}

void nsigii_filter_flash_destroy(NsigiiFilterFlash* flash) {
    free(flash);
}

double nsigii_filter_flash_compute_discriminant(NsigiiFilterFlash* flash) {
    if (!flash) return 0.0;
    return flash->b * flash->b - 4.0 * flash->a * flash->c;
}

NsigiiDiscriminantState nsigii_filter_flash_context_switch(NsigiiFilterFlash* flash) {
    if (!flash) return NSIGII_DISCRIMINANT_CHAOS;
    
    double delta = nsigii_filter_flash_compute_discriminant(flash);
    
    if (delta > 0) flash->state = NSIGII_DISCRIMINANT_ORDER;
    else if (delta == 0) flash->state = NSIGII_DISCRIMINANT_CONSENSUS;
    else flash->state = NSIGII_DISCRIMINANT_CHAOS;
    
    return flash->state;
}

bool nsigii_filter_flash_quadratic_roots(NsigiiFilterFlash* flash, double* root1, double* root2) {
    if (!flash || !root1 || !root2) return false;
    
    double delta = nsigii_filter_flash_compute_discriminant(flash);
    if (delta < 0) return false;
    
    double sqrt_delta = sqrt(delta);
    *root1 = (-flash->b + sqrt_delta) / (2.0 * flash->a);
    *root2 = (-flash->b - sqrt_delta) / (2.0 * flash->a);
    
    return true;
}

/* ============================================================================
 * ROPEN SPARSE DUPLEX ENCODING
 * ============================================================================ */
uint8_t nsigii_conjugate(uint8_t x) {
    return 0xF ^ x;
}

uint8_t* nsigii_rift_encode(const uint8_t* input, size_t len, bool polarity_a, NsigiiRBTree* tree, size_t* out_len) {
    if (!input || !out_len) return NULL;
    
    size_t output_len = len / 2 + (len % 2);
    uint8_t* output = (uint8_t*)malloc(output_len);
    if (!output) return NULL;
    
    for (size_t i = 0, j = 0; i < len; i += 2, j++) {
        uint8_t a = input[i];
        uint8_t b = (i + 1 < len) ? input[i + 1] : EPSILON_PAD;
        
        uint8_t logical;
        if (polarity_a) {
            logical = a ^ nsigii_conjugate(b);
        } else {
            logical = nsigii_conjugate(a) ^ b;
        }
        
        output[j] = logical;
        
        /* Insert into RB-AVL tree with confidence */
        if (tree) {
            uint8_t pol = polarity_a ? POLARITY_POS : POLARITY_NEG;
            nsigii_rbtree_insert(tree, (uint32_t)j, logical, 1.0, pol);
        }
    }
    
    *out_len = output_len;
    return output;
}

/* ============================================================================
 * TRIDENT CHANNEL IMPLEMENTATION
 * ============================================================================ */
NsigiiTridentChannel* nsigii_trident_channel_create(uint8_t id) {
    NsigiiTridentChannel* channel = (NsigiiTridentChannel*)calloc(1, sizeof(NsigiiTridentChannel));
    if (!channel) return NULL;
    
    channel->id = id;
    snprintf(channel->loopback_addr, sizeof(channel->loopback_addr), "127.0.0.%d", id + 1);
    channel->codec_ratio = (double)(id + 1) / 3.0;
    channel->state = NSIGII_STATE_ORDER;
    channel->tree = nsigii_rbtree_create();
    channel->flash = nsigii_filter_flash_create(1.0, 0.0, -1.0);
    
    return channel;
}

void nsigii_trident_channel_destroy(NsigiiTridentChannel* channel) {
    if (!channel) return;
    nsigii_rbtree_destroy(channel->tree);
    nsigii_filter_flash_destroy(channel->flash);
    free(channel);
}

NsigiiTridentPacket nsigii_channel_encode_message(NsigiiTridentChannel* channel, const uint8_t* content, size_t len) {
    NsigiiTridentPacket packet;
    memset(&packet, 0, sizeof(packet));
    
    if (!channel || !content) return packet;
    
    /* Header */
    packet.header.channel_id = NSIGII_CHANNEL_TRANSMITTER;
    packet.header.sequence_token = (uint32_t)time(NULL);
    packet.header.timestamp = (uint64_t)time(NULL);
    packet.header.codec_version = 1;
    
    /* Apply ROPEN sparse duplex encoding */
    size_t encoded_len;
    uint8_t* encoded = nsigii_rift_encode(content, len, true, channel->tree, &encoded_len);
    
    packet.payload.content = encoded;
    packet.payload.content_length = (uint32_t)encoded_len;
    
    /* Compute simple hash (first 32 bytes) */
    size_t hash_len = encoded_len < 32 ? encoded_len : 32;
    memcpy(packet.payload.message_hash, encoded, hash_len);
    
    /* Verification */
    packet.verification.rwx_flags = NSIGII_RWX_WRITE;
    strncpy(packet.verification.human_rights_tag, "NSIGII_HR_TRANSMIT", 31);
    
    /* Topology */
    packet.topology.wheel_position = 0;
    packet.topology.next_channel = NSIGII_CHANNEL_RECEIVER;
    
    return packet;
}

NsigiiTridentPacket nsigii_channel_decode_packet(NsigiiTridentChannel* channel, NsigiiTridentPacket packet) {
    if (!channel) return packet;
    
    /* Verify hash integrity */
    size_t hash_len = packet.payload.content_length < 32 ? packet.payload.content_length : 32;
    if (memcmp(packet.payload.content, packet.payload.message_hash, hash_len) != 0) {
        /* Hash mismatch - chaos state */
        channel->state = NSIGII_STATE_CHAOS;
    }
    
    /* Bipartite order check */
    if (packet.header.sequence_token % 2 == 0) {
        channel->state = NSIGII_STATE_ORDER;
    } else {
        channel->state = NSIGII_STATE_CHAOS;
    }
    
    /* Set RWX: Receiver has READ permission */
    packet.verification.rwx_flags = NSIGII_RWX_READ;
    
    /* Update topology */
    packet.topology.prev_channel = NSIGII_CHANNEL_TRANSMITTER;
    packet.topology.next_channel = NSIGII_CHANNEL_VERIFIER;
    packet.topology.wheel_position = 120;
    
    return packet;
}

double nsigii_bipartite_consensus(const uint8_t* content, size_t len, uint8_t wheel_pos) {
    if (!content || len == 0) return 0.0;
    
    /* Count set bits */
    int set_bits = 0;
    for (size_t i = 0; i < len; i++) {
        uint8_t byt = content[i];
        while (byt != 0) {
            set_bits += byt & 1;
            byt >>= 1;
        }
    }
    
    int total_bits = (int)len * 8;
    double base = (double)set_bits / (double)total_bits;
    double wheel_correct = sin((double)wheel_pos * M_PI / 180.0);
    double consensus = fabs(base + wheel_correct) / 2.0;
    
    return consensus;
}

bool nsigii_channel_verify_packet(NsigiiTridentChannel* channel, NsigiiTridentPacket* packet) {
    if (!channel || !packet) return false;
    
    /* Derive discriminant from bipartite consensus */
    double consensus = nsigii_bipartite_consensus(packet->payload.content, packet->payload.content_length, packet->topology.wheel_position);
    
    channel->flash->a = 1.0;
    channel->flash->b = consensus * 4.0;
    channel->flash->c = 1.0;
    
    NsigiiDiscriminantState state = nsigii_filter_flash_context_switch(channel->flash);
    
    switch (state) {
        case NSIGII_DISCRIMINANT_CONSENSUS:
            /* Flash point: consensus=0.5, B=2, Delta=0 */
            packet->verification.rwx_flags = NSIGII_RWX_FULL;
            packet->topology.wheel_position = 240;
            channel->state = NSIGII_STATE_VERIFIED;
            return true;
            
        case NSIGII_DISCRIMINANT_ORDER:
            /* Consensus > 0.5: coherent, grant full permissions */
            packet->verification.rwx_flags = NSIGII_RWX_FULL;
            packet->topology.wheel_position = 120;
            channel->state = NSIGII_STATE_ORDER;
            return true;
            
        case NSIGII_DISCRIMINANT_CHAOS:
            /* Consensus < 0.5: chaos state */
            packet->verification.rwx_flags = NSIGII_RWX_READ;
            channel->state = NSIGII_STATE_CHAOS;
            return false;
    }
    
    return false;
}

/* ============================================================================
 * COLOR SPACE CONVERSION
 * ============================================================================ */
uint8_t* nsigii_rgb_to_yuv420(const uint8_t* rgb, int width, int height) {
    if (!rgb) return NULL;
    
    size_t y_size = width * height;
    size_t uv_size = y_size / 4;
    uint8_t* yuv = (uint8_t*)malloc(y_size + 2 * uv_size);
    if (!yuv) return NULL;
    
    uint8_t* y_plane = yuv;
    uint8_t* u_plane = yuv + y_size;
    uint8_t* v_plane = u_plane + uv_size;
    
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            int idx = (j * width + i) * 3;
            double r = rgb[idx];
            double g = rgb[idx + 1];
            double b = rgb[idx + 2];
            
            double y = 0.299 * r + 0.587 * g + 0.114 * b;
            double u = -0.169 * r - 0.331 * g + 0.500 * b + 128.0;
            double v = 0.500 * r - 0.419 * g - 0.081 * b + 128.0;
            
            y_plane[j * width + i] = (uint8_t)(y < 0 ? 0 : (y > 255 ? 255 : y));
            
            /* Subsample U and V (4:2:0) */
            if (j % 2 == 0 && i % 2 == 0) {
                int uv_idx = (j / 2) * (width / 2) + (i / 2);
                u_plane[uv_idx] = (uint8_t)(u < 0 ? 0 : (u > 255 ? 255 : u));
                v_plane[uv_idx] = (uint8_t)(v < 0 ? 0 : (v > 255 ? 255 : v));
            }
        }
    }
    
    return yuv;
}

uint8_t* nsigii_yuv420_to_rgb(const uint8_t* yuv, int width, int height) {
    if (!yuv) return NULL;
    
    size_t y_size = width * height;
    size_t uv_size = y_size / 4;
    uint8_t* rgb = (uint8_t*)malloc(width * height * 3);
    if (!rgb) return NULL;
    
    const uint8_t* y_plane = yuv;
    const uint8_t* u_plane = yuv + y_size;
    const uint8_t* v_plane = u_plane + uv_size;
    
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            int y_idx = j * width + i;
            int uv_idx = (j / 2) * (width / 2) + (i / 2);
            
            double y = y_plane[y_idx];
            double u = u_plane[uv_idx] - 128.0;
            double v = v_plane[uv_idx] - 128.0;
            
            double r = y + 1.402 * v;
            double g = y - 0.344 * u - 0.714 * v;
            double b = y + 1.772 * u;
            
            int rgb_idx = (j * width + i) * 3;
            rgb[rgb_idx] = (uint8_t)(r < 0 ? 0 : (r > 255 ? 255 : r));
            rgb[rgb_idx + 1] = (uint8_t)(g < 0 ? 0 : (g > 255 ? 255 : g));
            rgb[rgb_idx + 2] = (uint8_t)(b < 0 ? 0 : (b > 255 ? 255 : b));
        }
    }
    
    return rgb;
}

/* ============================================================================
 * NSIGII CODEC CONTEXT
 * ============================================================================ */
NsigiiCodec* nsigii_codec_create(int width, int height) {
    NsigiiCodec* codec = (NsigiiCodec*)calloc(1, sizeof(NsigiiCodec));
    if (!codec) return NULL;
    
    codec->width = width;
    codec->height = height;
    codec->tree = nsigii_rbtree_create();
    codec->flash_buffer = nsigii_flash_buffer_create(width * height * 3);
    
    /* Initialize three trident channels */
    for (int i = 0; i < NSIGII_TRIDENT_CHANNELS; i++) {
        codec->channels[i] = nsigii_trident_channel_create((uint8_t)i);
    }
    
    return codec;
}

void nsigii_codec_destroy(NsigiiCodec* codec) {
    if (!codec) return;
    
    for (int i = 0; i < NSIGII_TRIDENT_CHANNELS; i++) {
        nsigii_trident_channel_destroy(codec->channels[i]);
    }
    
    nsigii_rbtree_destroy(codec->tree);
    nsigii_flash_buffer_destroy(codec->flash_buffer);
    free(codec);
}

/* ============================================================================
 * FILE I/O
 * ============================================================================ */
int nsigii_write_header(FILE* fp, const NsigiiFileHeader* header) {
    if (!fp || !header) return -1;
    
    fwrite(header->magic, 1, 8, fp);
    fwrite(header->version, 1, 8, fp);
    fwrite(&header->width, 4, 1, fp);
    fwrite(&header->height, 4, 1, fp);
    fwrite(&header->frame_count, 4, 1, fp);
    fwrite(&header->reserved, 4, 1, fp);
    
    return 0;
}

int nsigii_read_header(FILE* fp, NsigiiFileHeader* header) {
    if (!fp || !header) return -1;
    
    fread(header->magic, 1, 8, fp);
    fread(header->version, 1, 8, fp);
    fread(&header->width, 4, 1, fp);
    fread(&header->height, 4, 1, fp);
    fread(&header->frame_count, 4, 1, fp);
    fread(&header->reserved, 4, 1, fp);
    
    return 0;
}

int nsigii_write_frame(FILE* fp, const uint8_t* frame_data, size_t frame_len) {
    if (!fp || !frame_data) return -1;
    
    uint32_t len = (uint32_t)frame_len;
    fwrite(&len, 4, 1, fp);
    fwrite(frame_data, 1, frame_len, fp);
    
    return 0;
}

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */
const char* nsigii_version_string(void) {
    return NSIGII_VERSION_STRING;
}

const char* nsigii_discriminant_state_to_string(NsigiiDiscriminantState state) {
    switch (state) {
        case NSIGII_DISCRIMINANT_ORDER: return "ORDER";
        case NSIGII_DISCRIMINANT_CONSENSUS: return "CONSENSUS";
        case NSIGII_DISCRIMINANT_CHAOS: return "CHAOS";
        default: return "UNKNOWN";
    }
}

#ifdef __cplusplus
}
#endif
