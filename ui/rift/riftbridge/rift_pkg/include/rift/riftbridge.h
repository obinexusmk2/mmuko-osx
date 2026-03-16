#ifndef RIFTBRIDGE_H
#define RIFTBRIDGE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Trident topology constants
#define TRIDENT_DEGREE 3
#define TRIDENT_NODES_MAX 1000000
#define RIFT_PKG_VERSION "1.0.0"

// SemVerX channel types
typedef enum {
    CHANNEL_LEGACY,
    CHANNEL_EXPERIMENTAL, 
    CHANNEL_STABLE,
    CHANNEL_LTS
} SemVerXChannel;

// SemVerX version structure
typedef struct {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    SemVerXChannel channel;
} SemVerX;

// Trident node structure
typedef struct TridentNode {
    char* package_name;
    SemVerX version;
    struct TridentNode* incoming[2];  // Two incoming hooks
    struct TridentNode* outgoing[1];  // One outgoing hook
    bool is_bound;
    float confidence;
    uint8_t polarity;  // '+' or '-'
} TridentNode;

// Package registry entry
typedef struct {
    char* name;
    char* version_string;
    char* tarball_hash;
    char* signature;
    uint64_t timestamp;
    TridentNode* trident_node;
} PackageEntry;

// RIFT bridge context
typedef struct {
    void* local_registry;
    void* remote_registry;
    TridentNode** trident_topology;
    size_t trident_size;
    char* workspace_path;
    char* cache_path;
} RiftBridgeContext;

// Core API functions
RiftBridgeContext* riftbridge_init(const char* workspace_path);
void riftbridge_cleanup(RiftBridgeContext* ctx);

// Trident operations
TridentNode* trident_create_node(const char* package_name, const char* version_str);
bool trident_bind_node(TridentNode* node);
bool trident_resolve_diamond(TridentNode* app_node, const char* dependency_name);
void trident_prune_negative(TridentNode* topology[], size_t size);

// Package management
int riftbridge_install(RiftBridgeContext* ctx, const char* package_name, const char* version_spec);
int riftbridge_remove(RiftBridgeContext* ctx, const char* package_name);
int riftbridge_update(RiftBridgeContext* ctx, const char* package_name);

// Registry operations
int riftbridge_sync_local(RiftBridgeContext* ctx);
int riftbridge_sync_remote(RiftBridgeContext* ctx, const char* remote_url);
PackageEntry* riftbridge_query(RiftBridgeContext* ctx, const char* package_name);

// Encoding/decoding
size_t riftbridge_encode_package(const char* input_path, uint8_t* output, size_t output_cap, bool polarity_A);
size_t riftbridge_decode_package(const uint8_t* input, size_t input_len, char* output_path);

// Utility functions
SemVerX semverx_parse(const char* version_str);
char* semverx_format(SemVerX version);
bool semverx_satisfies(SemVerX version, const char* constraint);

#ifdef __cplusplus
}
#endif

#endif // RIFTBRIDGE_H