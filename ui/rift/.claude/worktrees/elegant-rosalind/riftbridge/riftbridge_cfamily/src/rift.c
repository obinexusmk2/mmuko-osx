#include "rift.h"
#include <stdlib.h>
#include <string.h>

// Like ropen.c conjugate
static uint8_t conjugate(uint8_t x) { return 0xF ^ x; }

// Polyglot encoding (extend ropen)
size_t rift_poly_encode(const uint8_t* in, size_t len, uint8_t* out, Polarity pol) {
    size_t out_len = 0;
    for (size_t i = 0; i < len; i += 2) {
        uint8_t a = in[i];
        uint8_t b = (i + 1 < len) ? in[i + 1] : 0x00;
        uint8_t logical = (pol == POLARITY_POS ? a : conjugate(a)) ^ (pol == POLARITY_POS ? conjugate(b) : b);
        out[out_len++] = logical;
    }
    return out_len;
}

// Tomography resolve: use trident for dependency "seeing"
int rift_tomography_resolve(eze* leader, uche knowledge, obi heart, iota shared, int rows, int cols) {
    // Check trident consensus
    if (leader->consensus < 2) return -1;  // No resolution

    // Scan row-col matrix for coherence (tomography "surface")
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (knowledge[r][c] != (int)shared) return -1;  // Polarity mismatch
        }
    }

    // Resolve to heart (obi)
    *heart = *leader->local;  // Assume local wins on consensus
    return 0;
}