/*
 * RIFT Pipeline Architecture - Stage 000 to 555
 * RIFT: RIFT Is a Flexible Translator
 * 
 * Pipeline Stages:
 *   000 - Tokenization (Lexical Analysis)
 *   001 - Process/Procedure Execution (NSIGII LTE Codec)
 *   111 - Semantic Processing (SP111)
 *   222 - Intermediate Representation
 *   333 - AST Target Program
 *   444 - Platform Target (macOS/Windows/Linux)
 *   555 - RIFTBridge (Polyglot C/C++/C#)
 * 
 * Author: OBINexus Computing
 * Version: 1.0.0
 */

#ifndef RIFT_PIPELINE_H
#define RIFT_PIPELINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * RIFT VERSION AND METADATA
 * ============================================================================ */
#define RIFT_VERSION_MAJOR 1
#define RIFT_VERSION_MINOR 0
#define RIFT_VERSION_PATCH 0
#define RIFT_VERSION_STRING "1.0.0"

/* Error message buffer size (shared across pipeline and bridge) */
#define RIFT_ERROR_MESSAGE_SIZE 512

/* ============================================================================
 * STAGE IDENTIFIERS (000-555)
 * ============================================================================ */
typedef enum {
    RIFT_STAGE_000 = 0,    /* Tokenization */
    RIFT_STAGE_001 = 1,    /* Process/Procedure */
    RIFT_STAGE_111 = 111,  /* Semantic Processing */
    RIFT_STAGE_222 = 222,  /* Intermediate Representation */
    RIFT_STAGE_333 = 333,  /* AST Target Program */
    RIFT_STAGE_444 = 444,  /* Platform Target */
    RIFT_STAGE_555 = 555,  /* RIFTBridge */
} RiftStage;

/* ============================================================================
 * TOKEN TRIPLET MODEL (Type, Value, Memory)
 * ============================================================================ */
typedef enum {
    TOKEN_TYPE_UNKNOWN = 0,
    TOKEN_TYPE_INT,
    TOKEN_TYPE_ROLE,
    TOKEN_TYPE_MASK,
    TOKEN_TYPE_OP,
    TOKEN_TYPE_QBYTE,      /* Quantum mode */
    TOKEN_TYPE_QROLE,      /* Quantum mode */
    TOKEN_TYPE_QMATRIX,    /* Quantum mode */
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_KEYWORD,
    TOKEN_TYPE_LITERAL,
    TOKEN_TYPE_OPERATOR,
    TOKEN_TYPE_DELIMITER,
    TOKEN_TYPE_EOF,
    TOKEN_TYPE_PATTERN_DOUBLE,  /* r"..." - static compile-time pattern */
    TOKEN_TYPE_PATTERN_SINGLE,  /* r'...' - dynamic runtime pattern */
    TOKEN_TYPE_MACRO_DEF        /* macro_rules! keyword */
} TokenType;

typedef union {
    int64_t i64;
    uint64_t u64;
    double f64;
    char* str;
    void* ptr;
    bool boolean;
} TokenValue;

typedef struct {
    uint32_t alignment;
    uint32_t size;
    uint32_t flags;
    void* memory_ptr;
} TokenMemory;

typedef struct {
    TokenType type;
    TokenValue value;
    TokenMemory memory;
    uint32_t validation_bits;  /* 0x01: allocated, 0x02: initialized, 0x04: locked */
} TokenTriplet;

/* ============================================================================
 * RIFT STAGE BOUND CONTEXT
 * ============================================================================ */
typedef struct {
    uint8_t stage_id;
    uint8_t process_id;
    uint8_t phase_id;
    void* stage_context;
} RiftStageBound;

/* ============================================================================
 * RIFT PIPELINE CONTEXT
 * ============================================================================ */
typedef struct RiftPipeline RiftPipeline;

struct RiftPipeline {
    RiftStage current_stage;
    RiftStageBound bound;
    TokenTriplet* tokens;
    size_t token_count;
    size_t token_capacity;
    void* ast_context;
    void* ir_context;
    void* target_context;
    
    /* Pipeline functions */
    int (*stage_000_tokenize)(RiftPipeline* pipeline, const char* input);
    int (*stage_001_process)(RiftPipeline* pipeline);
    int (*stage_111_semantic)(RiftPipeline* pipeline);
    int (*stage_222_ir)(RiftPipeline* pipeline);
    int (*stage_333_ast)(RiftPipeline* pipeline);
    int (*stage_444_target)(RiftPipeline* pipeline);
    int (*stage_555_bridge)(RiftPipeline* pipeline);
    
    /* Error handling */
    char error_message[RIFT_ERROR_MESSAGE_SIZE];
    int error_code;
};

/* ============================================================================
 * PIPELINE API
 * ============================================================================ */
RiftPipeline* rift_pipeline_create(void);
void rift_pipeline_destroy(RiftPipeline* pipeline);
int rift_pipeline_execute(RiftPipeline* pipeline, const char* input, RiftStage target_stage);
int rift_pipeline_execute_full(RiftPipeline* pipeline, const char* input);

/* Stage-specific functions */
int rift_stage_000_tokenize(RiftPipeline* pipeline, const char* input);
int rift_stage_001_process(RiftPipeline* pipeline);
int rift_stage_111_semantic(RiftPipeline* pipeline);
int rift_stage_222_ir(RiftPipeline* pipeline);
int rift_stage_333_ast(RiftPipeline* pipeline);
int rift_stage_444_target(RiftPipeline* pipeline);
int rift_stage_555_bridge(RiftPipeline* pipeline);

/* Utility functions */
const char* rift_stage_to_string(RiftStage stage);
const char* rift_token_type_to_string(TokenType type);

/* ============================================================================
 * FILE FORMAT TYPES
 * ============================================================================ */
typedef enum {
    RIFT_FORMAT_UNKNOWN = 0,
    RIFT_FORMAT_RIFT,      /* .rift - Full source (meta + semantic) */
    RIFT_FORMAT_RF,        /* .rf   - Semantic execution file */
    RIFT_FORMAT_META,      /* .meta - Token triplet metadata */
    RIFT_FORMAT_TOK,       /* .tok  - Tokenized stream output */
    RIFT_FORMAT_AST,       /* .ast  - AST serialized output */
    RIFT_FORMAT_C,         /* .c    - Generated C output */
} RiftFileFormat;

RiftFileFormat rift_detect_file_format(const char* filename);
const char* rift_file_format_to_string(RiftFileFormat format);

/* ============================================================================
 * CLI COMMAND TYPES
 * ============================================================================ */
typedef enum {
    RIFT_CMD_NONE = 0,
    RIFT_CMD_TOKENIZE,    /* rift tokenize <file> */
    RIFT_CMD_PARSE,       /* rift parse <file>    */
    RIFT_CMD_ANALYZE,     /* rift analyze <file>  */
    RIFT_CMD_GENERATE,    /* rift generate <file> */
    RIFT_CMD_EMIT,        /* rift emit <file>     */
    RIFT_CMD_COMPILE,     /* rift compile <file>  */
} RiftCommand;

/* Output extension for each subcommand */
const char* rift_command_output_ext(RiftCommand cmd);

#ifdef __cplusplus
}
#endif

#endif /* RIFT_PIPELINE_H */
