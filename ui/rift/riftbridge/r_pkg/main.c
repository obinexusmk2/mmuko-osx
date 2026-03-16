/* ropen.c – Rift Stage-3 Hex Encoder (MinGW-safe) */
#define RIFT_OPEN_MAIN
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t key;
    uint8_t val;
    float confidence;
} RBNode;

static uint8_t conjugate(uint8_t x) { return 0x0F ^ x; }

size_t rift_encode(const uint8_t *in, size_t in_len, uint8_t *out, bool polarity_A) {
    size_t out_len = 0;
    for (size_t i = 0; i < in_len; i += 2) {
        uint8_t a = in[i];
        uint8_t b = (i + 1 < in_len) ? in[i + 1] : 0x00; 
        uint8_t logical = (polarity_A ? a : conjugate(a)) ^ (polarity_A ? conjugate(b) : b);
        out[out_len++] = logical;
    }
    return out_len;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file> [A|B]\n", argv[0]);
        return 1;
    }

    bool A = (argc < 3 || argv[2][0] == 'A');
    FILE *f = fopen(argv[1], "rb");
    if (!f) return 1;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t *input = malloc(fsize);
    uint8_t *output = malloc(fsize); // Sparse duplex reduces size, so fsize is safe
    
    fread(input, 1, fsize, f);
    fclose(f);

    size_t n = rift_encode(input, fsize, output, A);
    printf("Encoded %zu bytes (C Implementation - Polarity %c)\n", n, A ? 'A' : 'B');
    
    for (size_t i = 0; i < n && i < 16; ++i) printf("%02X ", output[i]);
    printf("...\n");

    free(input);
    free(output);
    return 0;
}