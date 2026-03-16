#ifndef RIFT_H
#define RIFT_H

#include "riftbridge.h"
#include <stdio.h>
#include <stdlib.h>

// RIFT compiler stages
typedef enum {
    RIFT_STAGE_TOKENIZER = 0,
    RIFT_STAGE_PARSER = 1,
    RIFT_STAGE_AST = 2,
    RIFT_STAGE_VALIDATOR = 3,
    RIFT_STAGE_BYTECODE = 4,
    RIFT_STAGE_VERIFIER = 5,
    RIFT_STAGE_EMITTER = 6
} RiftStage;

// RIFT context for compilation
typedef struct {
    RiftStage current_stage;
    RiftBridgeContext* pkg_ctx;
    void* ast_nodes;
    void* bytecode;
    size_t cost_threshold;
    bool enable_audit;
    char* governance_file;
} RiftContext;

// Core RIFT functions
RiftContext* rift_init(const char* project_path);
void rift_cleanup(RiftContext* ctx);

// Stage processing
int rift_process_stage(RiftContext* ctx, RiftStage stage);
int rift_compile(RiftContext* ctx, const char* input_file, const char* output_file);

// Cost governance
float rift_calculate_cost(RiftContext* ctx);
bool rift_validate_cost(RiftContext* ctx, float cost);
void rift_apply_governance(RiftContext* ctx);

// Audit and telemetry
void rift_log_event(RiftContext* ctx, const char* event_type, const char* details);
char* rift_get_audit_trail(RiftContext* ctx);

// Memory management
void* rift_memory_alloc(RiftContext* ctx, size_t size);
void rift_memory_free(RiftContext* ctx, void* ptr);

// Error handling
typedef struct {
    int code;
    char* message;
    char* file;
    int line;
    RiftStage stage;
} RiftError;

RiftError* rift_get_last_error(RiftContext* ctx);
void rift_clear_error(RiftContext* ctx);

#endif // RIFT_H