/*
 * RIFT Stage 555: RIFTBridge Implementation
 * 
 * Polyglot bridge implementation for C, C++, and C# integration.
 * Provides unified pipeline execution across all three languages.
 */

#include "rift_555_bridge.h"
#include "../include/rift_pipeline.h"
#include "../nsigii-codec/nsigii_codec.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>

/* ============================================================================
 * FORWARD DECLARATIONS (from other stages)
 * ============================================================================ */
extern int rift_stage_000_tokenize(RiftPipeline* pipeline, const char* input);
extern int rift_stage_001_process(RiftPipeline* pipeline);
extern int rift_stage_333_ast(RiftPipeline* pipeline);
extern int rift_stage_444_target(RiftPipeline* pipeline);
extern void rift_000_print_tokens(const RiftPipeline* pipeline);
extern void rift_333_print_ast(const RiftPipeline* pipeline);
extern void rift_444_print_target(const RiftPipeline* pipeline);

/* ============================================================================
 * RIFTBRIDGE INTERNAL CONTEXT
 * ============================================================================ */
typedef struct {
    RiftPipeline* pipeline;
    NsigiiCodec* nsigii;
    int initialized;
} RiftBridgeInternal;

/* ============================================================================
 * TRIDENT PATTERN MATCHING
 * ============================================================================ */
bool rift_match_triplet(const char* pattern_a, void* capture_a,
                        const char* pattern_b, void* capture_b,
                        RiftAnchor anchor) {
    if (!pattern_a || !capture_a || !pattern_b || !capture_b) {
        return false;
    }
    
    regex_t regex_a, regex_b;
    bool result = true;
    
    /* Compile patterns */
    if (regcomp(&regex_a, pattern_a, REG_EXTENDED) != 0) {
        return false;
    }
    if (regcomp(&regex_b, pattern_b, REG_EXTENDED) != 0) {
        regfree(&regex_a);
        return false;
    }
    
    /* Get string values from captures */
    const char* str_a = *(const char**)capture_a;
    const char* str_b = *(const char**)capture_b;
    
    /* Match pattern A */
    if (str_a && regexec(&regex_a, str_a, 0, NULL, 0) != 0) {
        result = false;
    }
    
    /* Match pattern B */
    if (result && str_b && regexec(&regex_b, str_b, 0, NULL, 0) != 0) {
        result = false;
    }
    
    /* Apply anchor logic */
    if (result && anchor == RIFT_ANCHOR_END) {
        /* Check if patterns match at end of string */
        /* Simplified: just verify both matched */
    }
    
    regfree(&regex_a);
    regfree(&regex_b);
    
    return result;
}

/* ============================================================================
 * RIFTBRIDGE LIFECYCLE
 * ============================================================================ */
RiftBridge* riftbridge_create(RiftPolar polar) {
    RiftBridge* bridge = (RiftBridge*)calloc(1, sizeof(RiftBridge));
    if (!bridge) return NULL;
    
    bridge->polar = polar;
    bridge->version = (RIFTBRIDGE_VERSION_MAJOR << 16) | 
                      (RIFTBRIDGE_VERSION_MINOR << 8) | 
                      RIFTBRIDGE_VERSION_PATCH;
    
    /* Create internal context */
    RiftBridgeInternal* internal = (RiftBridgeInternal*)calloc(1, sizeof(RiftBridgeInternal));
    if (!internal) {
        free(bridge);
        return NULL;
    }
    
    /* Create pipeline */
    internal->pipeline = rift_pipeline_create();
    if (!internal->pipeline) {
        free(internal);
        free(bridge);
        return NULL;
    }
    
    bridge->stage_555 = internal;
    
    /* Set function pointers */
    bridge->initialize = riftbridge_initialize;
    bridge->tokenize = riftbridge_tokenize;
    bridge->process = riftbridge_process;
    bridge->build_ast = riftbridge_build_ast;
    bridge->generate_target = riftbridge_generate_target;
    bridge->execute_pipeline = riftbridge_execute_pipeline;
    bridge->destroy = (void (*)(RiftBridge*))riftbridge_destroy;
    bridge->match_triplet = rift_match_triplet;
    
    printf("[RIFT-555] RIFTBridge created (polar: %s)\n", rift_polar_to_string(polar));
    
    return bridge;
}

void riftbridge_destroy(RiftBridge* bridge) {
    if (!bridge) return;
    
    RiftBridgeInternal* internal = (RiftBridgeInternal*)bridge->stage_555;
    if (internal) {
        if (internal->pipeline) {
            rift_pipeline_destroy(internal->pipeline);
        }
        if (internal->nsigii) {
            nsigii_codec_destroy(internal->nsigii);
        }
        free(internal);
    }
    
    printf("[RIFT-555] RIFTBridge destroyed\n");
    free(bridge);
}

/* ============================================================================
 * RIFTBRIDGE INITIALIZATION
 * ============================================================================ */
int riftbridge_initialize(RiftBridge* bridge) {
    if (!bridge) return -1;
    
    RiftBridgeInternal* internal = (RiftBridgeInternal*)bridge->stage_555;
    if (!internal) return -1;
    
    printf("[RIFT-555] Initializing RIFTBridge...\n");
    
    /* Initialize NSIGII codec */
    internal->nsigii = nsigii_codec_create(64, 64);
    if (!internal->nsigii) {
        snprintf(bridge->error_message, sizeof(bridge->error_message),
                 "Failed to initialize NSIGII codec");
        bridge->error_code = -1;
        return -1;
    }
    
    internal->initialized = 1;
    
    printf("[RIFT-555] RIFTBridge initialized\n");
    return 0;
}

/* ============================================================================
 * PIPELINE STAGE WRAPPERS
 * ============================================================================ */
int riftbridge_tokenize(RiftBridge* bridge, const char* input) {
    if (!bridge || !input) return -1;
    
    RiftBridgeInternal* internal = (RiftBridgeInternal*)bridge->stage_555;
    if (!internal || !internal->pipeline) return -1;
    
    printf("[RIFT-555] Executing stage 000 (tokenize)...\n");
    
    int result = rift_stage_000_tokenize(internal->pipeline, input);
    
    if (result == 0) {
        bridge->stage_000 = internal->pipeline;
        rift_000_print_tokens(internal->pipeline);
    } else {
        snprintf(bridge->error_message, sizeof(bridge->error_message),
                 "Stage 000 failed: %s", internal->pipeline->error_message);
        bridge->error_code = internal->pipeline->error_code;
    }
    
    return result;
}

int riftbridge_process(RiftBridge* bridge) {
    if (!bridge) return -1;
    
    RiftBridgeInternal* internal = (RiftBridgeInternal*)bridge->stage_555;
    if (!internal || !internal->pipeline) return -1;
    
    printf("[RIFT-555] Executing stage 001 (process)...\n");
    
    int result = rift_stage_001_process(internal->pipeline);
    
    if (result == 0) {
        bridge->stage_001 = internal->pipeline;
    } else {
        snprintf(bridge->error_message, sizeof(bridge->error_message),
                 "Stage 001 failed: %s", internal->pipeline->error_message);
        bridge->error_code = internal->pipeline->error_code;
    }
    
    return result;
}

int riftbridge_build_ast(RiftBridge* bridge) {
    if (!bridge) return -1;
    
    RiftBridgeInternal* internal = (RiftBridgeInternal*)bridge->stage_555;
    if (!internal || !internal->pipeline) return -1;
    
    printf("[RIFT-555] Executing stage 333 (AST)...\n");
    
    int result = rift_stage_333_ast(internal->pipeline);
    
    if (result == 0) {
        bridge->stage_333 = internal->pipeline->ast_context;
        rift_333_print_ast(internal->pipeline);
    } else {
        snprintf(bridge->error_message, sizeof(bridge->error_message),
                 "Stage 333 failed: %s", internal->pipeline->error_message);
        bridge->error_code = internal->pipeline->error_code;
    }
    
    return result;
}

int riftbridge_generate_target(RiftBridge* bridge) {
    if (!bridge) return -1;
    
    RiftBridgeInternal* internal = (RiftBridgeInternal*)bridge->stage_555;
    if (!internal || !internal->pipeline) return -1;
    
    printf("[RIFT-555] Executing stage 444 (target)...\n");
    
    int result = rift_stage_444_target(internal->pipeline);
    
    if (result == 0) {
        bridge->stage_444 = internal->pipeline->target_context;
        rift_444_print_target(internal->pipeline);
    } else {
        snprintf(bridge->error_message, sizeof(bridge->error_message),
                 "Stage 444 failed: %s", internal->pipeline->error_message);
        bridge->error_code = internal->pipeline->error_code;
    }
    
    return result;
}

/* ============================================================================
 * FULL PIPELINE EXECUTION
 * ============================================================================ */
int riftbridge_execute_pipeline(RiftBridge* bridge, const char* input) {
    if (!bridge || !input) return -1;
    
    printf("\n========================================\n");
    printf("[RIFT-555] Executing Full Pipeline\n");
    printf("========================================\n\n");
    
    /* Stage 000: Tokenize */
    if (riftbridge_tokenize(bridge, input) != 0) {
        return -1;
    }
    
    /* Stage 001: Process */
    if (riftbridge_process(bridge) != 0) {
        return -1;
    }
    
    /* Stage 333: AST */
    if (riftbridge_build_ast(bridge) != 0) {
        return -1;
    }
    
    /* Stage 444: Target */
    if (riftbridge_generate_target(bridge) != 0) {
        return -1;
    }
    
    printf("\n========================================\n");
    printf("[RIFT-555] Pipeline Execution Complete\n");
    printf("========================================\n");
    
    return 0;
}

/* ============================================================================
 * NSIGII CODEC INTEGRATION
 * ============================================================================ */
void* riftbridge_get_nsigii_codec(RiftBridge* bridge) {
    if (!bridge) return NULL;
    
    RiftBridgeInternal* internal = (RiftBridgeInternal*)bridge->stage_555;
    if (!internal) return NULL;
    
    return internal->nsigii;
}

int riftbridge_encode_nsigii(RiftBridge* bridge, const uint8_t* input, 
                              size_t len, uint8_t** output, size_t* out_len) {
    if (!bridge || !input || !output || !out_len) return -1;
    
    RiftBridgeInternal* internal = (RiftBridgeInternal*)bridge->stage_555;
    if (!internal || !internal->nsigii) return -1;
    
    /* Use NSIGII codec for encoding */
    NsigiiRBTree* tree = nsigii_rbtree_create();
    if (!tree) return -1;
    
    size_t encoded_len;
    uint8_t* encoded = nsigii_rift_encode(input, len, true, tree, &encoded_len);
    
    nsigii_rbtree_destroy(tree);
    
    if (!encoded) return -1;
    
    *output = encoded;
    *out_len = encoded_len;
    
    return 0;
}

int riftbridge_decode_nsigii(RiftBridge* bridge, const uint8_t* input,
                              size_t len, uint8_t** output, size_t* out_len) {
    if (!bridge || !input || !output || !out_len) return -1;
    
    /* Simplified decode - would need full implementation */
    *output = (uint8_t*)malloc(len * 2);
    if (!*output) return -1;
    
    /* Placeholder: just copy for now */
    memcpy(*output, input, len);
    *out_len = len;
    
    return 0;
}

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */
const char* riftbridge_version_string(void) {
    return RIFTBRIDGE_VERSION_STRING;
}

const char* rift_polar_to_string(RiftPolar polar) {
    switch (polar) {
        case RIFT_POLAR_C: return "C";
        case RIFT_POLAR_CPP: return "C++";
        case RIFT_POLAR_CS: return "C#";
        default: return "Unknown";
    }
}

/* ============================================================================
 * RIFT PIPELINE API IMPLEMENTATION
 * ============================================================================ */
RiftPipeline* rift_pipeline_create(void) {
    RiftPipeline* pipeline = (RiftPipeline*)calloc(1, sizeof(RiftPipeline));
    if (!pipeline) return NULL;
    
    pipeline->current_stage = RIFT_STAGE_000;
    pipeline->token_capacity = 0;
    pipeline->token_count = 0;
    pipeline->tokens = NULL;
    
    return pipeline;
}

void rift_pipeline_destroy(RiftPipeline* pipeline) {
    if (!pipeline) return;
    
    /* Free tokens */
    if (pipeline->tokens) {
        for (size_t i = 0; i < pipeline->token_count; i++) {
            if (pipeline->tokens[i].value.str && 
                (pipeline->tokens[i].validation_bits & 0x04)) {
                free(pipeline->tokens[i].value.str);
            }
        }
        free(pipeline->tokens);
    }
    
    /* Free AST context */
    if (pipeline->ast_context) {
        /* Would call rift_333_destroy_ast in full implementation */
    }
    
    /* Free target context */
    if (pipeline->target_context) {
        free(pipeline->target_context);
    }
    
    free(pipeline);
}

int rift_pipeline_execute(RiftPipeline* pipeline, const char* input, RiftStage target_stage) {
    if (!pipeline || !input) return -1;
    
    /* Execute stages up to target */
    if (target_stage >= RIFT_STAGE_000) {
        if (rift_stage_000_tokenize(pipeline, input) != 0) return -1;
    }
    
    if (target_stage >= RIFT_STAGE_001) {
        if (rift_stage_001_process(pipeline) != 0) return -1;
    }
    
    if (target_stage >= RIFT_STAGE_333) {
        if (rift_stage_333_ast(pipeline) != 0) return -1;
    }
    
    if (target_stage >= RIFT_STAGE_444) {
        if (rift_stage_444_target(pipeline) != 0) return -1;
    }
    
    return 0;
}

int rift_pipeline_execute_full(RiftPipeline* pipeline, const char* input) {
    return rift_pipeline_execute(pipeline, input, RIFT_STAGE_444);
}

const char* rift_stage_to_string(RiftStage stage) {
    switch (stage) {
        case RIFT_STAGE_000: return "000 (Tokenization)";
        case RIFT_STAGE_001: return "001 (Process/Procedure)";
        case RIFT_STAGE_111: return "111 (Semantic)";
        case RIFT_STAGE_222: return "222 (IR)";
        case RIFT_STAGE_333: return "333 (AST)";
        case RIFT_STAGE_444: return "444 (Target)";
        case RIFT_STAGE_555: return "555 (Bridge)";
        default: return "Unknown";
    }
}

const char* rift_token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_TYPE_UNKNOWN: return "UNKNOWN";
        case TOKEN_TYPE_INT: return "INT";
        case TOKEN_TYPE_ROLE: return "ROLE";
        case TOKEN_TYPE_MASK: return "MASK";
        case TOKEN_TYPE_OP: return "OP";
        case TOKEN_TYPE_QBYTE: return "QBYTE";
        case TOKEN_TYPE_QROLE: return "QROLE";
        case TOKEN_TYPE_QMATRIX: return "QMATRIX";
        case TOKEN_TYPE_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_TYPE_KEYWORD: return "KEYWORD";
        case TOKEN_TYPE_LITERAL: return "LITERAL";
        case TOKEN_TYPE_OPERATOR: return "OPERATOR";
        case TOKEN_TYPE_DELIMITER: return "DELIMITER";
        case TOKEN_TYPE_EOF: return "EOF";
        case TOKEN_TYPE_PATTERN_DOUBLE: return "PATTERN_DOUBLE";
        case TOKEN_TYPE_PATTERN_SINGLE: return "PATTERN_SINGLE";
        case TOKEN_TYPE_MACRO_DEF: return "MACRO_DEF";
        default: return "INVALID";
    }
}

/* ============================================================================
 * FILE FORMAT DETECTION
 * ============================================================================ */
RiftFileFormat rift_detect_file_format(const char* filename) {
    if (!filename) return RIFT_FORMAT_UNKNOWN;
    const char* dot = strrchr(filename, '.');
    if (!dot) return RIFT_FORMAT_UNKNOWN;

    if (strcmp(dot, ".rift") == 0) return RIFT_FORMAT_RIFT;
    if (strcmp(dot, ".rf") == 0)   return RIFT_FORMAT_RF;
    if (strcmp(dot, ".meta") == 0) return RIFT_FORMAT_META;
    if (strcmp(dot, ".tok") == 0)  return RIFT_FORMAT_TOK;
    if (strcmp(dot, ".ast") == 0)  return RIFT_FORMAT_AST;
    if (strcmp(dot, ".c") == 0)    return RIFT_FORMAT_C;
    return RIFT_FORMAT_UNKNOWN;
}

const char* rift_file_format_to_string(RiftFileFormat format) {
    switch (format) {
        case RIFT_FORMAT_RIFT:    return "RIFT source (.rift)";
        case RIFT_FORMAT_RF:      return "RF semantic (.rf)";
        case RIFT_FORMAT_META:    return "Meta triplet (.meta)";
        case RIFT_FORMAT_TOK:     return "Token stream (.tok)";
        case RIFT_FORMAT_AST:     return "AST (.ast)";
        case RIFT_FORMAT_C:       return "C source (.c)";
        case RIFT_FORMAT_UNKNOWN:
        default:                  return "Unknown";
    }
}

const char* rift_command_output_ext(RiftCommand cmd) {
    switch (cmd) {
        case RIFT_CMD_TOKENIZE:  return ".tok";
        case RIFT_CMD_PARSE:     return ".ast";
        case RIFT_CMD_ANALYZE:   return ".tast";
        case RIFT_CMD_GENERATE:  return ".c";
        case RIFT_CMD_EMIT:      return ".c";
        case RIFT_CMD_COMPILE:   return ".exe";
        case RIFT_CMD_NONE:
        default:                 return NULL;
    }
}
