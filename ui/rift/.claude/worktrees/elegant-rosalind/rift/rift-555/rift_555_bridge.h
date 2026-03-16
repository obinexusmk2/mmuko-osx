/*
 * RIFT Stage 555: RIFTBridge
 * 
 * Polyglot bridge header for C, C++, and C# integration.
 * This header provides a unified interface across all three languages.
 * 
 * Usage:
 *   C:      #include "rift_555_bridge.h"
 *   C++:    #include "rift_555_bridge.hpp" (wraps this header)
 *   C#:     Use RiftBridge.cs (source generator compatible)
 */

#ifndef RIFT_555_BRIDGE_H
#define RIFT_555_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Error message buffer size (matches rift_pipeline.h) */
#ifndef RIFT_ERROR_MESSAGE_SIZE
#define RIFT_ERROR_MESSAGE_SIZE 512
#endif

/* ============================================================================
 * RIFTBRIDGE VERSION
 * ============================================================================ */
#define RIFTBRIDGE_VERSION_MAJOR 1
#define RIFTBRIDGE_VERSION_MINOR 0
#define RIFTBRIDGE_VERSION_PATCH 0
#define RIFTBRIDGE_VERSION_STRING "1.0.0"

/* ============================================================================
 * TRIDENT POLAR DEFINITIONS
 * ============================================================================ */
typedef enum {
    RIFT_POLAR_C = 0,       /* Procedural C */
    RIFT_POLAR_CPP = 1,     /* Generic C++ */
    RIFT_POLAR_CS = 2       /* Managed C# */
} RiftPolar;

/* ============================================================================
 * RIFT HEX PATTERN (Trident Anchor)
 * ============================================================================ */
typedef struct {
    char args;      /* [A-Z] - Alpha token */
    int vargs;      /* [0-9] - Numeric token */
} RiftHexPattern;

/* RIFT_HEX macro for C */
#define RIFT_HEX(args, vargs) \
    rift_match_triplet( \
        "[A-Z]", &(args), \
        "[0-9]", &(vargs), \
        RIFT_ANCHOR_END \
    )

/* Anchor types */
typedef enum {
    RIFT_ANCHOR_START = 0,
    RIFT_ANCHOR_END = 1,
    RIFT_ANCHOR_WORD = 2,
    RIFT_ANCHOR_LINE = 3
} RiftAnchor;

/* ============================================================================
 * TOKEN TRIPLET (Cross-language compatible)
 * ============================================================================ */
typedef struct {
    uint32_t type;           /* Token type */
    union {
        int64_t i64;
        uint64_t u64;
        double f64;
        char* str;
        void* ptr;
    } value;
    struct {
        uint32_t alignment;
        uint32_t size;
        uint32_t flags;
    } memory;
} RiftTriplet;

/* ============================================================================
 * RIFTBRIDGE CONTEXT
 * ============================================================================ */
typedef struct RiftBridge RiftBridge;

struct RiftBridge {
    RiftPolar polar;
    uint32_t version;
    
    /* Pipeline stages */
    void* stage_000;  /* Tokenizer */
    void* stage_001;  /* Processor */
    void* stage_111;  /* Semantic */
    void* stage_222;  /* IR */
    void* stage_333;  /* AST */
    void* stage_444;  /* Target */
    void* stage_555;  /* Bridge (self) */
    
    /* NSIGII codec integration */
    void* nsigii_codec;
    
    /* Error handling */
    char error_message[RIFT_ERROR_MESSAGE_SIZE];
    int error_code;
    
    /* Function pointers */
    int (*initialize)(RiftBridge* bridge);
    int (*tokenize)(RiftBridge* bridge, const char* input);
    int (*process)(RiftBridge* bridge);
    int (*build_ast)(RiftBridge* bridge);
    int (*generate_target)(RiftBridge* bridge);
    int (*execute_pipeline)(RiftBridge* bridge, const char* input);
    void (*destroy)(RiftBridge* bridge);
    
    /* Trident pattern matching */
    bool (*match_triplet)(const char* pattern_a, void* capture_a,
                          const char* pattern_b, void* capture_b,
                          RiftAnchor anchor);
};

/* ============================================================================
 * RIFTBRIDGE API
 * ============================================================================ */

/* Lifecycle */
RiftBridge* riftbridge_create(RiftPolar polar);
void riftbridge_destroy(RiftBridge* bridge);

/* Pipeline execution */
int riftbridge_initialize(RiftBridge* bridge);
int riftbridge_tokenize(RiftBridge* bridge, const char* input);
int riftbridge_process(RiftBridge* bridge);
int riftbridge_build_ast(RiftBridge* bridge);
int riftbridge_generate_target(RiftBridge* bridge);
int riftbridge_execute_pipeline(RiftBridge* bridge, const char* input);

/* Trident pattern matching */
bool rift_match_triplet(const char* pattern_a, void* capture_a,
                        const char* pattern_b, void* capture_b,
                        RiftAnchor anchor);

/* NSIGII codec integration */
void* riftbridge_get_nsigii_codec(RiftBridge* bridge);
int riftbridge_encode_nsigii(RiftBridge* bridge, const uint8_t* input, 
                              size_t len, uint8_t** output, size_t* out_len);
int riftbridge_decode_nsigii(RiftBridge* bridge, const uint8_t* input,
                              size_t len, uint8_t** output, size_t* out_len);

/* Utility */
const char* riftbridge_version_string(void);
const char* rift_polar_to_string(RiftPolar polar);

#ifdef __cplusplus
} /* extern "C" */

/* ============================================================================
 * C++ WRAPPER INTERFACE (outside extern "C" for template support)
 * ============================================================================ */
#include <utility>
#include <string>

/* Template-based interface for C++ */
template<typename T, typename U>
class RiftBridgeCpp {
public:
    RiftBridgeCpp() : bridge_(riftbridge_create(RIFT_POLAR_CPP)) {}
    ~RiftBridgeCpp() { riftbridge_destroy(bridge_); }

    bool initialize() { return riftbridge_initialize(bridge_) == 0; }
    bool tokenize(const char* input) { return riftbridge_tokenize(bridge_, input) == 0; }
    bool process() { return riftbridge_process(bridge_) == 0; }
    bool buildAst() { return riftbridge_build_ast(bridge_) == 0; }
    bool generateTarget() { return riftbridge_generate_target(bridge_) == 0; }
    bool executePipeline(const char* input) {
        return riftbridge_execute_pipeline(bridge_, input) == 0;
    }

    /* Template-based pattern matching */
    std::pair<T, U> matchHexPattern(const std::string& input) {
        std::pair<T, U> result;
        /* Implementation would use regex matching */
        return result;
    }

private:
    RiftBridge* bridge_;
};

#endif /* __cplusplus */

/* ============================================================================
 * C# COMPATIBILITY MACROS
 * ============================================================================ */
#ifdef RIFT_CS_BUILD
/* These macros help with C# source generator compatibility */
#define RIFT_ATTRIBUTE(attr) __attribute__((annotate(#attr)))
#define RIFT_GENERATED_REGEX(pattern) 
#else
#define RIFT_ATTRIBUTE(attr)
#define RIFT_GENERATED_REGEX(pattern)
#endif

#endif /* RIFT_555_BRIDGE_H */
