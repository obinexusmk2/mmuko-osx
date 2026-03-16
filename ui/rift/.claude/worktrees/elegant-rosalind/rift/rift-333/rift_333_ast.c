/*
 * RIFT Stage 333: AST Target Program
 * 
 * This stage builds an Abstract Syntax Tree from processed tokens.
 * The AST represents the target program structure before platform-specific
 * code generation.
 */

#include "../include/rift_pipeline.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * AST NODE TYPES
 * ============================================================================ */
typedef enum {
    AST_NODE_PROGRAM = 0,
    AST_NODE_FUNCTION,
    AST_NODE_VARIABLE,
    AST_NODE_EXPRESSION,
    AST_NODE_STATEMENT,
    AST_NODE_BLOCK,
    AST_NODE_IF,
    AST_NODE_WHILE,
    AST_NODE_FOR,
    AST_NODE_RETURN,
    AST_NODE_CALL,
    AST_NODE_BINARY_OP,
    AST_NODE_UNARY_OP,
    AST_NODE_LITERAL,
    AST_NODE_IDENTIFIER,
    AST_NODE_TYPE,
    AST_NODE_PARAMETER,
    AST_NODE_ARGUMENT,
    AST_NODE_SPAN,        /* RIFT-specific: memory span */
    AST_NODE_ALIGN,       /* RIFT-specific: alignment directive */
    AST_NODE_ROLE,        /* RIFT-specific: role definition */
    AST_NODE_MASK         /* RIFT-specific: mask operation */
} AstNodeType;

/* ============================================================================
 * AST NODE STRUCTURE
 * ============================================================================ */
typedef struct AstNode AstNode;

struct AstNode {
    AstNodeType type;
    TokenTriplet* token;           /* Associated token */
    
    /* Node data */
    union {
        /* For literals */
        struct {
            int64_t int_val;
            double float_val;
            char* string_val;
        } literal;
        
        /* For identifiers */
        struct {
            char* name;
            void* symbol;
        } identifier;
        
        /* For binary operations */
        struct {
            char* operator;
            AstNode* left;
            AstNode* right;
        } binary;
        
        /* For unary operations */
        struct {
            char* operator;
            AstNode* operand;
        } unary;
        
        /* For functions */
        struct {
            char* name;
            AstNode** params;
            size_t param_count;
            AstNode* body;
            AstNode* return_type;
        } function;
        
        /* For variables */
        struct {
            char* name;
            AstNode* var_type;
            AstNode* initializer;
            uint32_t flags;
        } variable;
        
        /* For calls */
        struct {
            AstNode* callee;
            AstNode** args;
            size_t arg_count;
        } call;
        
        /* For blocks */
        struct {
            AstNode** statements;
            size_t statement_count;
        } block;
        
        /* For control flow */
        struct {
            AstNode* condition;
            AstNode* then_branch;
            AstNode* else_branch;
        } conditional;
        
        /* For RIFT-specific spans */
        struct {
            uint32_t alignment;
            uint32_t size;
            uint32_t direction;  /* right->left or left->right */
            bool is_continuous;
            bool is_mutable;
        } span;
    } data;
    
    /* Tree structure */
    AstNode* parent;
    AstNode** children;
    size_t child_count;
    size_t child_capacity;
    
    /* Metadata */
    uint32_t line;
    uint32_t column;
    uint32_t stage_bound;  /* RIFT stage that created this node */
};

/* ============================================================================
 * AST CONTEXT
 * ============================================================================ */
typedef struct {
    AstNode* root;
    AstNode** node_pool;
    size_t node_count;
    size_t node_capacity;
    size_t current_token;
    TokenTriplet* tokens;
    size_t token_count;
} AstContext;

/* ============================================================================
 * AST NODE LIFECYCLE
 * ============================================================================ */
static AstNode* ast_node_create(AstNodeType type) {
    AstNode* node = (AstNode*)calloc(1, sizeof(AstNode));
    if (!node) return NULL;
    
    node->type = type;
    node->child_capacity = 4;
    node->children = (AstNode**)malloc(node->child_capacity * sizeof(AstNode*));
    if (!node->children) {
        free(node);
        return NULL;
    }
    
    node->stage_bound = 333;
    return node;
}

static void ast_node_destroy(AstNode* node) {
    if (!node) return;
    
    /* Free children recursively */
    for (size_t i = 0; i < node->child_count; i++) {
        ast_node_destroy(node->children[i]);
    }
    free(node->children);
    
    /* Free node-specific data */
    switch (node->type) {
        case AST_NODE_LITERAL:
            if (node->data.literal.string_val) {
                free(node->data.literal.string_val);
            }
            break;
        case AST_NODE_IDENTIFIER:
            if (node->data.identifier.name) {
                free(node->data.identifier.name);
            }
            break;
        case AST_NODE_BINARY_OP:
            if (node->data.binary.operator) {
                free(node->data.binary.operator);
            }
            break;
        case AST_NODE_UNARY_OP:
            if (node->data.unary.operator) {
                free(node->data.unary.operator);
            }
            break;
        case AST_NODE_FUNCTION:
            if (node->data.function.name) {
                free(node->data.function.name);
            }
            free(node->data.function.params);
            break;
        case AST_NODE_VARIABLE:
            if (node->data.variable.name) {
                free(node->data.variable.name);
            }
            break;
        default:
            break;
    }
    
    free(node);
}

static void ast_node_add_child(AstNode* parent, AstNode* child) {
    if (!parent || !child) return;
    
    /* Expand children array if needed */
    if (parent->child_count >= parent->child_capacity) {
        size_t new_capacity = parent->child_capacity * 2;
        AstNode** new_children = (AstNode**)realloc(
            parent->children, new_capacity * sizeof(AstNode*));
        if (!new_children) return;
        
        parent->children = new_children;
        parent->child_capacity = new_capacity;
    }
    
    parent->children[parent->child_count++] = child;
    child->parent = parent;
}

/* ============================================================================
 * PARSER HELPERS
 * ============================================================================ */
static TokenTriplet* peek_token(AstContext* ctx) {
    if (ctx->current_token >= ctx->token_count) return NULL;
    return &ctx->tokens[ctx->current_token];
}

static TokenTriplet* consume_token(AstContext* ctx) {
    if (ctx->current_token >= ctx->token_count) return NULL;
    return &ctx->tokens[ctx->current_token++];
}

static bool match_token_type(AstContext* ctx, TokenType type) {
    TokenTriplet* token = peek_token(ctx);
    return token && token->type == type;
}

static bool match_keyword(AstContext* ctx, const char* keyword) {
    TokenTriplet* token = peek_token(ctx);
    if (!token || token->type != TOKEN_TYPE_KEYWORD) return false;
    return token->value.str && strcmp(token->value.str, keyword) == 0;
}

static TokenTriplet* expect_token(AstContext* ctx, TokenType type) {
    TokenTriplet* token = peek_token(ctx);
    if (!token || token->type != type) {
        return NULL;
    }
    return consume_token(ctx);
}

/* ============================================================================
 * PARSER RULES
 * ============================================================================ */

/* Forward declarations */
static AstNode* parse_expression(AstContext* ctx);
static AstNode* parse_statement(AstContext* ctx);
static AstNode* parse_block(AstContext* ctx);

/* Parse literal */
static AstNode* parse_literal(AstContext* ctx) {
    TokenTriplet* token = peek_token(ctx);
    if (!token || token->type != TOKEN_TYPE_LITERAL) return NULL;
    
    consume_token(ctx);
    
    AstNode* node = ast_node_create(AST_NODE_LITERAL);
    if (!node) return NULL;
    
    node->token = token;
    node->data.literal.int_val = token->value.i64;
    node->data.literal.float_val = token->value.f64;
    
    return node;
}

/* Parse identifier */
static AstNode* parse_identifier(AstContext* ctx) {
    TokenTriplet* token = peek_token(ctx);
    if (!token || token->type != TOKEN_TYPE_IDENTIFIER) return NULL;
    
    consume_token(ctx);
    
    AstNode* node = ast_node_create(AST_NODE_IDENTIFIER);
    if (!node) return NULL;
    
    node->token = token;
    if (token->value.str) {
        node->data.identifier.name = strdup(token->value.str);
    }
    
    return node;
}

/* Parse primary expression */
static AstNode* parse_primary(AstContext* ctx) {
    /* Try literal */
    AstNode* literal = parse_literal(ctx);
    if (literal) return literal;
    
    /* Try identifier */
    AstNode* identifier = parse_identifier(ctx);
    if (identifier) return identifier;
    
    /* Try parenthesized expression */
    if (match_token_type(ctx, TOKEN_TYPE_DELIMITER)) {
        TokenTriplet* token = peek_token(ctx);
        if (token->value.str && strcmp(token->value.str, "(") == 0) {
            consume_token(ctx);
            AstNode* expr = parse_expression(ctx);
            expect_token(ctx, TOKEN_TYPE_DELIMITER);  /* Expect ')' */
            return expr;
        }
    }
    
    return NULL;
}

/* Parse binary expression (simplified) */
static AstNode* parse_expression(AstContext* ctx) {
    AstNode* left = parse_primary(ctx);
    if (!left) return NULL;
    
    /* Check for binary operator */
    TokenTriplet* op_token = peek_token(ctx);
    if (op_token && op_token->type == TOKEN_TYPE_OPERATOR) {
        consume_token(ctx);
        
        AstNode* right = parse_primary(ctx);
        if (!right) {
            ast_node_destroy(left);
            return NULL;
        }
        
        AstNode* binop = ast_node_create(AST_NODE_BINARY_OP);
        if (!binop) {
            ast_node_destroy(left);
            ast_node_destroy(right);
            return NULL;
        }
        
        binop->data.binary.operator = op_token->value.str ? strdup(op_token->value.str) : NULL;
        binop->data.binary.left = left;
        binop->data.binary.right = right;
        
        ast_node_add_child(binop, left);
        ast_node_add_child(binop, right);
        
        return binop;
    }
    
    return left;
}

/* Parse variable declaration */
static AstNode* parse_variable_declaration(AstContext* ctx) {
    /* Check for type keyword */
    if (!match_token_type(ctx, TOKEN_TYPE_KEYWORD)) return NULL;
    
    TokenTriplet* type_token = consume_token(ctx);
    
    /* Expect identifier */
    AstNode* name_node = parse_identifier(ctx);
    if (!name_node) return NULL;
    
    AstNode* var = ast_node_create(AST_NODE_VARIABLE);
    if (!var) {
        ast_node_destroy(name_node);
        return NULL;
    }
    
    var->data.variable.name = name_node->data.identifier.name;
    name_node->data.identifier.name = NULL;  /* Transfer ownership */
    ast_node_destroy(name_node);
    
    /* Check for initializer */
    if (match_token_type(ctx, TOKEN_TYPE_OPERATOR)) {
        TokenTriplet* op = peek_token(ctx);
        if (op->value.str && strcmp(op->value.str, "=") == 0) {
            consume_token(ctx);
            var->data.variable.initializer = parse_expression(ctx);
        }
    }
    
    /* Expect semicolon */
    expect_token(ctx, TOKEN_TYPE_DELIMITER);
    
    return var;
}

/* Parse statement */
static AstNode* parse_statement(AstContext* ctx) {
    /* Try variable declaration */
    AstNode* var = parse_variable_declaration(ctx);
    if (var) return var;
    
    /* Try expression statement */
    AstNode* expr = parse_expression(ctx);
    if (expr) {
        expect_token(ctx, TOKEN_TYPE_DELIMITER);  /* Semicolon */
        return expr;
    }
    
    return NULL;
}

/* Parse block */
static AstNode* parse_block(AstContext* ctx) {
    /* Expect '{' */
    TokenTriplet* open = peek_token(ctx);
    if (!open || open->type != TOKEN_TYPE_DELIMITER || 
        !open->value.str || strcmp(open->value.str, "{") != 0) {
        return NULL;
    }
    consume_token(ctx);
    
    AstNode* block = ast_node_create(AST_NODE_BLOCK);
    if (!block) return NULL;
    
    /* Parse statements until '}' */
    while (ctx->current_token < ctx->token_count) {
        TokenTriplet* close = peek_token(ctx);
        if (close && close->type == TOKEN_TYPE_DELIMITER &&
            close->value.str && strcmp(close->value.str, "}") == 0) {
            consume_token(ctx);
            break;
        }
        
        AstNode* stmt = parse_statement(ctx);
        if (stmt) {
            ast_node_add_child(block, stmt);
        } else {
            /* Skip unknown token */
            consume_token(ctx);
        }
    }
    
    return block;
}

/* Parse function */
static AstNode* parse_function(AstContext* ctx) {
    /* Check for function keyword or type + identifier pattern */
    if (!match_token_type(ctx, TOKEN_TYPE_KEYWORD)) return NULL;
    
    TokenTriplet* type_token = consume_token(ctx);
    
    /* Expect function name */
    AstNode* name_node = parse_identifier(ctx);
    if (!name_node) return NULL;
    
    /* Expect '(' */
    if (!expect_token(ctx, TOKEN_TYPE_DELIMITER)) {
        ast_node_destroy(name_node);
        return NULL;
    }
    
    /* Parse parameters */
    AstNode** params = NULL;
    size_t param_count = 0;
    
    /* Expect ')' */
    expect_token(ctx, TOKEN_TYPE_DELIMITER);
    
    /* Parse body */
    AstNode* body = parse_block(ctx);
    
    AstNode* func = ast_node_create(AST_NODE_FUNCTION);
    if (!func) {
        ast_node_destroy(name_node);
        ast_node_destroy(body);
        free(params);
        return NULL;
    }
    
    func->data.function.name = name_node->data.identifier.name;
    name_node->data.identifier.name = NULL;
    ast_node_destroy(name_node);
    
    func->data.function.params = params;
    func->data.function.param_count = param_count;
    func->data.function.body = body;
    
    if (body) {
        ast_node_add_child(func, body);
    }
    
    return func;
}

/* Parse program */
static AstNode* parse_program(AstContext* ctx) {
    AstNode* program = ast_node_create(AST_NODE_PROGRAM);
    if (!program) return NULL;
    
    while (ctx->current_token < ctx->token_count) {
        TokenTriplet* token = peek_token(ctx);
        if (!token || token->type == TOKEN_TYPE_EOF) break;
        
        AstNode* decl = NULL;
        
        /* Try function */
        decl = parse_function(ctx);
        if (decl) {
            ast_node_add_child(program, decl);
            continue;
        }
        
        /* Try variable declaration */
        decl = parse_variable_declaration(ctx);
        if (decl) {
            ast_node_add_child(program, decl);
            continue;
        }
        
        /* Try statement */
        decl = parse_statement(ctx);
        if (decl) {
            ast_node_add_child(program, decl);
            continue;
        }
        
        /* Skip unknown */
        consume_token(ctx);
    }
    
    return program;
}

/* ============================================================================
 * RIFT STAGE 333: AST BUILD
 * ============================================================================ */
int rift_stage_333_ast(RiftPipeline* pipeline) {
    if (!pipeline || !pipeline->tokens) return -1;
    
    printf("\n[RIFT-333] AST Target Program building...\n");
    
    /* Initialize AST context */
    AstContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.tokens = pipeline->tokens;
    ctx.token_count = pipeline->token_count;
    ctx.current_token = 0;
    
    /* Parse program */
    ctx.root = parse_program(&ctx);
    if (!ctx.root) {
        snprintf(pipeline->error_message, sizeof(pipeline->error_message),
                 "Failed to build AST");
        pipeline->error_code = -1;
        return -1;
    }
    
    /* Store AST in pipeline */
    pipeline->ast_context = ctx.root;
    
    /* Update stage bound */
    pipeline->bound.stage_id = 3;
    pipeline->bound.process_id = 3;
    pipeline->bound.phase_id = 3;
    
    pipeline->current_stage = RIFT_STAGE_333;
    printf("[RIFT-333] AST built with %zu top-level nodes\n", ctx.root->child_count);
    
    return 0;
}

/* ============================================================================
 * AST OUTPUT
 * ============================================================================ */
static void print_ast_node(const AstNode* node, int indent) {
    if (!node) return;
    
    const char* type_names[] = {
        "PROGRAM", "FUNCTION", "VARIABLE", "EXPRESSION", "STATEMENT",
        "BLOCK", "IF", "WHILE", "FOR", "RETURN", "CALL",
        "BINARY_OP", "UNARY_OP", "LITERAL", "IDENTIFIER", "TYPE",
        "PARAMETER", "ARGUMENT", "SPAN", "ALIGN", "ROLE", "MASK"
    };
    
    for (int i = 0; i < indent; i++) printf("  ");
    
    printf("%s", type_names[node->type]);
    
    switch (node->type) {
        case AST_NODE_LITERAL:
            printf(" (value: %ld)", (long)node->data.literal.int_val);
            break;
        case AST_NODE_IDENTIFIER:
            if (node->data.identifier.name) {
                printf(" (name: %s)", node->data.identifier.name);
            }
            break;
        case AST_NODE_FUNCTION:
            if (node->data.function.name) {
                printf(" (name: %s)", node->data.function.name);
            }
            break;
        case AST_NODE_VARIABLE:
            if (node->data.variable.name) {
                printf(" (name: %s)", node->data.variable.name);
            }
            break;
        case AST_NODE_BINARY_OP:
            if (node->data.binary.operator) {
                printf(" (op: %s)", node->data.binary.operator);
            }
            break;
        default:
            break;
    }
    
    printf("\n");
    
    for (size_t i = 0; i < node->child_count; i++) {
        print_ast_node(node->children[i], indent + 1);
    }
}

void rift_333_print_ast(const RiftPipeline* pipeline) {
    if (!pipeline || !pipeline->ast_context) return;
    
    printf("\n=== RIFT-333 AST ===\n");
    print_ast_node((AstNode*)pipeline->ast_context, 0);
    printf("\n");
}

/* ============================================================================
 * AST CLEANUP
 * ============================================================================ */
void rift_333_destroy_ast(RiftPipeline* pipeline) {
    if (!pipeline || !pipeline->ast_context) return;
    
    ast_node_destroy((AstNode*)pipeline->ast_context);
    pipeline->ast_context = NULL;
}
