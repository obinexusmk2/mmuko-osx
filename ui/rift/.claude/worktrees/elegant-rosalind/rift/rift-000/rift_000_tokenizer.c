/*
 * RIFT Stage 000: Tokenization Pipeline
 * 
 * This stage implements lexical analysis using DFA-based tokenization.
 * Output: TokenTriplets (type, value, memory) for stage 001.
 */

#include "../include/rift_pipeline.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <regex.h>

/* ============================================================================
 * TOKENIZER STATE MACHINE
 * ============================================================================ */
typedef enum {
    STATE_START = 0,
    STATE_IDENTIFIER,
    STATE_NUMBER,
    STATE_STRING,
    STATE_OPERATOR,
    STATE_DELIMITER,
    STATE_WHITESPACE,
    STATE_COMMENT,
    STATE_EOF
} TokenizerState;

typedef struct {
    const char* pattern;
    TokenType type;
} TokenPattern;

/* RIFT Token Patterns (R-syntax compatible) */
static const TokenPattern token_patterns[] = {
    /* Keywords */
    {"^align$", TOKEN_TYPE_KEYWORD},
    {"^span$", TOKEN_TYPE_KEYWORD},
    {"^vector$", TOKEN_TYPE_KEYWORD},
    {"^tuple$", TOKEN_TYPE_KEYWORD},
    {"^array$", TOKEN_TYPE_KEYWORD},
    {"^map$", TOKEN_TYPE_KEYWORD},
    {"^dsa$", TOKEN_TYPE_KEYWORD},
    {"^int$", TOKEN_TYPE_KEYWORD},
    {"^role$", TOKEN_TYPE_KEYWORD},
    {"^mask$", TOKEN_TYPE_KEYWORD},
    {"^op$", TOKEN_TYPE_KEYWORD},
    {"^qbyte$", TOKEN_TYPE_KEYWORD},
    {"^qrole$", TOKEN_TYPE_KEYWORD},
    {"^qmatrix$", TOKEN_TYPE_KEYWORD},
    
    /* Identifiers */
    {"^[a-zA-Z_][a-zA-Z0-9_]*$", TOKEN_TYPE_IDENTIFIER},
    
    /* Numbers */
    {"^[0-9]+$", TOKEN_TYPE_LITERAL},
    {"^0x[0-9a-fA-F]+$", TOKEN_TYPE_LITERAL},
    {"^[0-9]+\\.[0-9]+$", TOKEN_TYPE_LITERAL},
    
    /* Operators */
    {"^\\+$", TOKEN_TYPE_OPERATOR},
    {"^-$", TOKEN_TYPE_OPERATOR},
    {"^\\*$", TOKEN_TYPE_OPERATOR},
    {"^/$", TOKEN_TYPE_OPERATOR},
    {"^=$", TOKEN_TYPE_OPERATOR},
    {"^<$", TOKEN_TYPE_OPERATOR},
    {"^>$", TOKEN_TYPE_OPERATOR},
    {"^\\^$", TOKEN_TYPE_OPERATOR},
    {"^\\|$", TOKEN_TYPE_OPERATOR},
    {"^&$", TOKEN_TYPE_OPERATOR},
    
    /* Delimiters */
    {"^\\{$", TOKEN_TYPE_DELIMITER},
    {"^\\}$", TOKEN_TYPE_DELIMITER},
    {"^\\[$", TOKEN_TYPE_DELIMITER},
    {"^\\]$", TOKEN_TYPE_DELIMITER},
    {"^\\($", TOKEN_TYPE_DELIMITER},
    {"^\\)$", TOKEN_TYPE_DELIMITER},
    {"^;$", TOKEN_TYPE_DELIMITER},
    {"^,$", TOKEN_TYPE_DELIMITER},
    {"^\\.$", TOKEN_TYPE_DELIMITER},
    
    /* Whitespace */
    {"^\\s+$", TOKEN_TYPE_UNKNOWN},
    
    {NULL, TOKEN_TYPE_UNKNOWN}
};

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */
static char* extract_token(const char* input, size_t start, size_t len) {
    char* token = (char*)malloc(len + 1);
    if (!token) return NULL;
    strncpy(token, input + start, len);
    token[len] = '\0';
    return token;
}

static bool match_pattern(const char* text, const char* pattern) {
    regex_t regex;
    int result;
    
    result = regcomp(&regex, pattern, REG_EXTENDED);
    if (result) return false;
    
    result = regexec(&regex, text, 0, NULL, 0);
    regfree(&regex);
    
    return result == 0;
}

static TokenType identify_token_type(const char* token_text) {
    for (int i = 0; token_patterns[i].pattern != NULL; i++) {
        if (match_pattern(token_text, token_patterns[i].pattern)) {
            return token_patterns[i].type;
        }
    }
    return TOKEN_TYPE_UNKNOWN;
}

static size_t get_token_length(const char* input, size_t pos) {
    size_t len = 0;
    char c = input[pos];
    
    /* Identifier or keyword */
    if (isalpha(c) || c == '_') {
        while (isalnum(input[pos + len]) || input[pos + len] == '_') {
            len++;
        }
        return len;
    }
    
    /* Number */
    if (isdigit(c)) {
        /* Hex number */
        if (c == '0' && (input[pos + 1] == 'x' || input[pos + 1] == 'X')) {
            len = 2;
            while (isxdigit(input[pos + len])) len++;
            return len;
        }
        /* Decimal number */
        while (isdigit(input[pos + len])) len++;
        if (input[pos + len] == '.') {
            len++;
            while (isdigit(input[pos + len])) len++;
        }
        return len;
    }
    
    /* String literal */
    if (c == '"' || c == '\'') {
        char quote = c;
        len = 1;
        while (input[pos + len] != '\0' && input[pos + len] != quote) {
            if (input[pos + len] == '\\' && input[pos + len + 1] != '\0') {
                len += 2;
            } else {
                len++;
            }
        }
        if (input[pos + len] == quote) len++;
        return len;
    }
    
    /* Multi-char operators */
    if (c == '=' && input[pos + 1] == '=') return 2;
    if (c == '!' && input[pos + 1] == '=') return 2;
    if (c == '<' && input[pos + 1] == '=') return 2;
    if (c == '>' && input[pos + 1] == '=') return 2;
    if (c == '&' && input[pos + 1] == '&') return 2;
    if (c == '|' && input[pos + 1] == '|') return 2;
    if (c == '+' && input[pos + 1] == '+') return 2;
    if (c == '-' && input[pos + 1] == '-') return 2;
    if (c == '-' && input[pos + 1] == '>') return 2;
    
    /* Single char tokens */
    if (strchr("+-*/=<>^|&{}[]();,.", c) != NULL) return 1;
    
    /* Whitespace */
    if (isspace(c)) {
        while (isspace(input[pos + len])) len++;
        return len;
    }
    
    /* Comment */
    if (c == '/' && input[pos + 1] == '/') {
        while (input[pos + len] != '\0' && input[pos + len] != '\n') len++;
        return len;
    }
    if (c == '/' && input[pos + 1] == '*') {
        len = 2;
        while (input[pos + len] != '\0' && !(input[pos + len] == '*' && input[pos + len + 1] == '/')) {
            len++;
        }
        if (input[pos + len] == '*') len += 2;
        return len;
    }
    
    /* Unknown - advance by 1 */
    return 1;
}

/* ============================================================================
 * RIFT STAGE 000: TOKENIZE
 * ============================================================================ */
int rift_stage_000_tokenize(RiftPipeline* pipeline, const char* input) {
    if (!pipeline || !input) return -1;
    
    printf("[RIFT-000] Tokenization started...\n");
    
    /* Initialize token storage */
    pipeline->token_capacity = 1024;
    pipeline->tokens = (TokenTriplet*)calloc(pipeline->token_capacity, sizeof(TokenTriplet));
    if (!pipeline->tokens) {
        snprintf(pipeline->error_message, sizeof(pipeline->error_message), 
                 "Failed to allocate token buffer");
        pipeline->error_code = -1;
        return -1;
    }
    
    pipeline->token_count = 0;
    size_t pos = 0;
    size_t input_len = strlen(input);
    
    while (pos < input_len && input[pos] != '\0') {
        /* Skip whitespace but track it */
        if (isspace(input[pos])) {
            size_t ws_len = 0;
            while (isspace(input[pos + ws_len])) ws_len++;
            pos += ws_len;
            continue;
        }
        
        /* Skip comments */
        if (input[pos] == '/' && input[pos + 1] == '/') {
            while (pos < input_len && input[pos] != '\n') pos++;
            continue;
        }
        if (input[pos] == '/' && input[pos + 1] == '*') {
            pos += 2;
            while (pos < input_len && !(input[pos] == '*' && input[pos + 1] == '/')) pos++;
            pos += 2;
            continue;
        }
        
        /* Get token length */
        size_t token_len = get_token_length(input, pos);
        if (token_len == 0) {
            pos++;
            continue;
        }
        
        /* Extract token text */
        char* token_text = extract_token(input, pos, token_len);
        if (!token_text) {
            snprintf(pipeline->error_message, sizeof(pipeline->error_message),
                     "Memory allocation failed at position %zu", pos);
            pipeline->error_code = -2;
            return -1;
        }
        
        /* Identify token type */
        TokenType type = identify_token_type(token_text);
        
        /* Skip whitespace tokens */
        if (type != TOKEN_TYPE_UNKNOWN) {
            /* Expand token array if needed */
            if (pipeline->token_count >= pipeline->token_capacity) {
                size_t new_capacity = pipeline->token_capacity * 2;
                TokenTriplet* new_tokens = (TokenTriplet*)realloc(
                    pipeline->tokens, new_capacity * sizeof(TokenTriplet));
                if (!new_tokens) {
                    free(token_text);
                    snprintf(pipeline->error_message, sizeof(pipeline->error_message),
                             "Failed to expand token buffer");
                    pipeline->error_code = -3;
                    return -1;
                }
                pipeline->tokens = new_tokens;
                pipeline->token_capacity = new_capacity;
            }
            
            /* Create token triplet */
            TokenTriplet* token = &pipeline->tokens[pipeline->token_count];
            token->type = type;
            token->validation_bits = 0x03;  /* allocated | initialized */
            
            /* Set value based on type */
            switch (type) {
                case TOKEN_TYPE_LITERAL:
                    if (strncmp(token_text, "0x", 2) == 0) {
                        token->value.u64 = strtoull(token_text + 2, NULL, 16);
                    } else if (strchr(token_text, '.') != NULL) {
                        token->value.f64 = strtod(token_text, NULL);
                    } else {
                        token->value.i64 = strtoll(token_text, NULL, 10);
                    }
                    break;
                    
                case TOKEN_TYPE_IDENTIFIER:
                case TOKEN_TYPE_KEYWORD:
                    token->value.str = token_text;
                    token->validation_bits |= 0x04;  /* locked (owns memory) */
                    break;
                    
                default:
                    token->value.str = token_text;
                    token->validation_bits |= 0x04;
                    break;
            }
            
            /* Set memory info */
            token->memory.alignment = 8;
            token->memory.size = token_len;
            token->memory.flags = 0;
            token->memory.memory_ptr = (void*)token_text;
            
            pipeline->token_count++;
        } else {
            free(token_text);
        }
        
        pos += token_len;
    }
    
    /* Add EOF token */
    if (pipeline->token_count < pipeline->token_capacity) {
        TokenTriplet* eof_token = &pipeline->tokens[pipeline->token_count];
        eof_token->type = TOKEN_TYPE_EOF;
        eof_token->value.str = NULL;
        eof_token->memory.alignment = 0;
        eof_token->memory.size = 0;
        eof_token->memory.flags = 0;
        eof_token->memory.memory_ptr = NULL;
        eof_token->validation_bits = 0x03;
        pipeline->token_count++;
    }
    
    pipeline->current_stage = RIFT_STAGE_000;
    printf("[RIFT-000] Tokenization complete: %zu tokens generated\n", pipeline->token_count);
    
    return 0;
}

/* ============================================================================
 * DEBUG OUTPUT
 * ============================================================================ */
void rift_000_print_tokens(const RiftPipeline* pipeline) {
    if (!pipeline || !pipeline->tokens) return;
    
    printf("\n=== RIFT-000 Token Stream ===\n");
    printf("%-4s %-20s %-30s %-10s\n", "Idx", "Type", "Value", "MemSize");
    printf("%-4s %-20s %-30s %-10s\n", "---", "----", "-----", "-------");
    
    for (size_t i = 0; i < pipeline->token_count; i++) {
        const TokenTriplet* t = &pipeline->tokens[i];
        const char* type_str = rift_token_type_to_string(t->type);
        
        char value_str[64];
        switch (t->type) {
            case TOKEN_TYPE_LITERAL:
                snprintf(value_str, sizeof(value_str), "%ld", (long)t->value.i64);
                break;
            case TOKEN_TYPE_IDENTIFIER:
            case TOKEN_TYPE_KEYWORD:
                snprintf(value_str, sizeof(value_str), "%s", 
                        t->value.str ? t->value.str : "(null)");
                break;
            case TOKEN_TYPE_OPERATOR:
            case TOKEN_TYPE_DELIMITER:
                snprintf(value_str, sizeof(value_str), "%s",
                        t->value.str ? t->value.str : "(null)");
                break;
            default:
                snprintf(value_str, sizeof(value_str), "-");
                break;
        }
        
        printf("%-4zu %-20s %-30s %-10u\n", i, type_str, value_str, t->memory.size);
    }
    printf("\n");
}
