#include "riftest.h"
#include <stdio.h>

// Test tomography
void riftest_tomography(eze* leader, uche matrix, int rows, int cols) {
    obi heart = NULL;
    iota shared = POLARITY_POS;
    if (rift_tomography_resolve(leader, matrix, heart, shared, rows, cols) == 0) {
        printf("Tomography resolved!\n");
    } else {
        printf("Resolution failed.\n");
    }
}

// Test poly encode
void riftest_poly_encode(const char* input) {
    uint8_t out[1024];
    size_t len = rift_poly_encode((uint8_t*)input, strlen(input), out, POLARITY_POS);
    printf("Encoded %zu bytes: ", len);
    for (size_t i = 0; i < len; i++) printf("%02X ", out[i]);
    printf("\n");
}