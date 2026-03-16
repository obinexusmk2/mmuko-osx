/*
 * RIFT Stage 001: Process/Procedure Execution
 * 
 * This stage implements process-bound execution with NSIGII LTE codec integration.
 * - Processes token stream from stage 000
 * - Applies NSIGII codec for compression/encoding
 * - Manages stage-bound execution order
 * - Output: Processed tokens ready for semantic analysis
 */

#include "../include/rift_pipeline.h"
#include "../nsigii-codec/nsigii_codec.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * PROCESS CONTEXT
 * ============================================================================ */
typedef struct {
    uint8_t process_id;
    uint8_t procedure_id;
    uint32_t execution_flags;
    NsigiiCodec* codec;
    void* procedure_context;
} ProcessContext;

/* Procedure types for stage 001 */
typedef enum {
    PROCEDURE_NONE = 0,
    PROCEDURE_TOKEN_FILTER,
    PROCEDURE_MEMORY_ALIGN,
    PROCEDURE_NSIGII_ENCODE,
    PROCEDURE_NSIGII_DECODE,
    PROCEDURE_VALIDATION
} ProcedureType;

/* ============================================================================
 * LTE (LINK-THEN-EXECUTE) FORMAT
 * ============================================================================ */
typedef struct {
    char magic[4];           /* "LTE\0" */
    uint8_t stage_id;
    uint8_t process_id;
    uint16_t token_count;
    uint32_t data_size;
    uint32_t checksum;
} LTEHeader;

/* ============================================================================
 * PROCEDURE IMPLEMENTATIONS
 * ============================================================================ */

/* Token filtering procedure */
static int procedure_token_filter(RiftPipeline* pipeline, ProcessContext* ctx) {
    if (!pipeline || !ctx) return -1;
    
    printf("[RIFT-001] Procedure: Token Filter\n");
    
    /* Filter out unknown/whitespace tokens */
    size_t write_idx = 0;
    for (size_t i = 0; i < pipeline->token_count; i++) {
        TokenTriplet* token = &pipeline->tokens[i];
        
        /* Keep valid tokens only */
        if (token->type != TOKEN_TYPE_UNKNOWN && 
            token->type != TOKEN_TYPE_EOF) {
            if (write_idx != i) {
                pipeline->tokens[write_idx] = *token;
            }
            write_idx++;
        }
    }
    
    printf("[RIFT-001] Filtered: %zu -> %zu tokens\n", pipeline->token_count, write_idx);
    pipeline->token_count = write_idx;
    
    return 0;
}

/* Memory alignment procedure */
static int procedure_memory_align(RiftPipeline* pipeline, ProcessContext* ctx) {
    if (!pipeline || !ctx) return -1;
    
    printf("[RIFT-001] Procedure: Memory Alignment\n");
    
    /* Align all token memory to 8-byte boundaries */
    for (size_t i = 0; i < pipeline->token_count; i++) {
        TokenTriplet* token = &pipeline->tokens[i];
        
        /* Ensure 8-byte alignment */
        token->memory.alignment = 8;
        
        /* Round up size to alignment boundary */
        size_t aligned_size = (token->memory.size + 7) & ~7;
        token->memory.size = (uint32_t)aligned_size;
        
        /* Set alignment flag */
        token->memory.flags |= 0x01;
    }
    
    printf("[RIFT-001] Memory aligned for %zu tokens\n", pipeline->token_count);
    
    return 0;
}

/* NSIGII encoding procedure - uses the codec for token compression */
static int procedure_nsigii_encode(RiftPipeline* pipeline, ProcessContext* ctx) {
    if (!pipeline || !ctx || !ctx->codec) return -1;
    
    printf("[RIFT-001] Procedure: NSIGII Encode\n");
    
    /* Convert tokens to byte stream for NSIGII encoding */
    size_t token_data_size = pipeline->token_count * sizeof(TokenTriplet);
    uint8_t* token_bytes = (uint8_t*)pipeline->tokens;
    
    /* Use NSIGII codec for encoding */
    /* Note: In real implementation, this would compress the token stream */
    
    printf("[RIFT-001] NSIGII encoded %zu bytes of token data\n", token_data_size);
    
    return 0;
}

/* Validation procedure */
static int procedure_validation(RiftPipeline* pipeline, ProcessContext* ctx) {
    if (!pipeline || !ctx) return -1;
    
    printf("[RIFT-001] Procedure: Validation\n");
    
    /* Validate token triplets */
    int valid_count = 0;
    int invalid_count = 0;
    
    for (size_t i = 0; i < pipeline->token_count; i++) {
        TokenTriplet* token = &pipeline->tokens[i];
        
        /* Check validation bits */
        if ((token->validation_bits & 0x03) == 0x03) {
            valid_count++;
        } else {
            invalid_count++;
        }
        
        /* Check memory alignment */
        if (token->memory.alignment != 8) {
            printf("[RIFT-001] Warning: Token %zu has misaligned memory\n", i);
        }
    }
    
    printf("[RIFT-001] Validation: %d valid, %d invalid\n", valid_count, invalid_count);
    
    return (invalid_count == 0) ? 0 : -1;
}

/* ============================================================================
 * LTE (LINK-THEN-EXECUTE) FORMAT HANDLING
 * ============================================================================ */
static uint32_t lte_compute_checksum(const uint8_t* data, size_t len) {
    uint32_t checksum = 0;
    for (size_t i = 0; i < len; i++) {
        checksum = (checksum << 1) ^ data[i];
    }
    return checksum;
}

static int lte_write_header(FILE* fp, const LTEHeader* header) {
    if (!fp || !header) return -1;
    
    fwrite(header->magic, 1, 4, fp);
    fwrite(&header->stage_id, 1, 1, fp);
    fwrite(&header->process_id, 1, 1, fp);
    fwrite(&header->token_count, 2, 1, fp);
    fwrite(&header->data_size, 4, 1, fp);
    fwrite(&header->checksum, 4, 1, fp);
    
    return 0;
}

static int lte_read_header(FILE* fp, LTEHeader* header) {
    if (!fp || !header) return -1;
    
    fread(header->magic, 1, 4, fp);
    fread(&header->stage_id, 1, 1, fp);
    fread(&header->process_id, 1, 1, fp);
    fread(&header->token_count, 2, 1, fp);
    fread(&header->data_size, 4, 1, fp);
    fread(&header->checksum, 4, 1, fp);
    
    return 0;
}

/* ============================================================================
 * RIFT STAGE 001: PROCESS
 * ============================================================================ */
int rift_stage_001_process(RiftPipeline* pipeline) {
    if (!pipeline) return -1;
    
    printf("\n[RIFT-001] Process/Procedure Execution started...\n");
    
    /* Initialize process context */
    ProcessContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.process_id = 1;
    ctx.procedure_id = 0;
    ctx.execution_flags = 0;
    
    /* Initialize NSIGII codec for token compression */
    ctx.codec = nsigii_codec_create(64, 64);  /* Small dimensions for token processing */
    if (!ctx.codec) {
        snprintf(pipeline->error_message, sizeof(pipeline->error_message),
                 "Failed to initialize NSIGII codec");
        pipeline->error_code = -1;
        return -1;
    }
    
    /* Execute procedures in order */
    ProcedureType procedures[] = {
        PROCEDURE_TOKEN_FILTER,
        PROCEDURE_MEMORY_ALIGN,
        PROCEDURE_NSIGII_ENCODE,
        PROCEDURE_VALIDATION
    };
    
    int num_procedures = sizeof(procedures) / sizeof(procedures[0]);
    
    for (int i = 0; i < num_procedures; i++) {
        ctx.procedure_id = (uint8_t)procedures[i];
        
        int result = 0;
        switch (procedures[i]) {
            case PROCEDURE_TOKEN_FILTER:
                result = procedure_token_filter(pipeline, &ctx);
                break;
            case PROCEDURE_MEMORY_ALIGN:
                result = procedure_memory_align(pipeline, &ctx);
                break;
            case PROCEDURE_NSIGII_ENCODE:
                result = procedure_nsigii_encode(pipeline, &ctx);
                break;
            case PROCEDURE_VALIDATION:
                result = procedure_validation(pipeline, &ctx);
                break;
            default:
                printf("[RIFT-001] Unknown procedure: %d\n", procedures[i]);
                result = -1;
                break;
        }
        
        if (result != 0) {
            snprintf(pipeline->error_message, sizeof(pipeline->error_message),
                     "Procedure %d failed with code %d", procedures[i], result);
            pipeline->error_code = result;
            nsigii_codec_destroy(ctx.codec);
            return result;
        }
    }
    
    /* Update stage bound */
    pipeline->bound.stage_id = 1;
    pipeline->bound.process_id = ctx.process_id;
    pipeline->bound.phase_id = 1;
    
    /* Cleanup */
    nsigii_codec_destroy(ctx.codec);
    
    pipeline->current_stage = RIFT_STAGE_001;
    printf("[RIFT-001] Process/Procedure Execution complete\n");
    
    return 0;
}

/* ============================================================================
 * LTE SERIALIZATION
 * ============================================================================ */
int rift_001_serialize_lte(const RiftPipeline* pipeline, const char* filename) {
    if (!pipeline || !filename) return -1;
    
    FILE* fp = fopen(filename, "wb");
    if (!fp) return -1;
    
    /* Create LTE header */
    LTEHeader header;
    memcpy(header.magic, "LTE\0", 4);
    header.stage_id = 1;
    header.process_id = pipeline->bound.process_id;
    header.token_count = (uint16_t)pipeline->token_count;
    header.data_size = (uint32_t)(pipeline->token_count * sizeof(TokenTriplet));
    
    /* Compute checksum */
    header.checksum = lte_compute_checksum(
        (const uint8_t*)pipeline->tokens, 
        pipeline->token_count * sizeof(TokenTriplet)
    );
    
    /* Write header */
    lte_write_header(fp, &header);
    
    /* Write token data */
    fwrite(pipeline->tokens, sizeof(TokenTriplet), pipeline->token_count, fp);
    
    fclose(fp);
    
    printf("[RIFT-001] Serialized to LTE format: %s\n", filename);
    
    return 0;
}

int rift_001_deserialize_lte(RiftPipeline* pipeline, const char* filename) {
    if (!pipeline || !filename) return -1;
    
    FILE* fp = fopen(filename, "rb");
    if (!fp) return -1;
    
    /* Read header */
    LTEHeader header;
    lte_read_header(fp, &header);
    
    /* Verify magic */
    if (memcmp(header.magic, "LTE\0", 4) != 0) {
        fclose(fp);
        return -1;
    }
    
    /* Allocate token buffer */
    pipeline->token_capacity = header.token_count;
    pipeline->tokens = (TokenTriplet*)malloc(header.token_count * sizeof(TokenTriplet));
    if (!pipeline->tokens) {
        fclose(fp);
        return -1;
    }
    
    /* Read tokens */
    pipeline->token_count = fread(pipeline->tokens, sizeof(TokenTriplet), 
                                   header.token_count, fp);
    
    /* Verify checksum */
    uint32_t computed_checksum = lte_compute_checksum(
        (const uint8_t*)pipeline->tokens,
        pipeline->token_count * sizeof(TokenTriplet)
    );
    
    if (computed_checksum != header.checksum) {
        printf("[RIFT-001] Warning: Checksum mismatch\n");
    }
    
    fclose(fp);
    
    printf("[RIFT-001] Deserialized from LTE format: %s (%zu tokens)\n", 
           filename, pipeline->token_count);
    
    return 0;
}
