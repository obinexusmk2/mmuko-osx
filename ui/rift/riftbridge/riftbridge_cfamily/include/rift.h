#ifndef RIFT_H
#define RIFT_H

#include <stdint.h>

// SemVerX struct with channel and polarity
typedef enum { CHANNEL_LEGACY, CHANNEL_EXPERIMENTAL, CHANNEL_STABLE } Channel;
typedef enum { POLARITY_POS = '+', POLARITY_NEG = '-' } Polarity;

typedef struct SemVerX {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    Channel channel;
    Polarity polarity;
} SemVerX;

// Eze (leader/power) struct for trident
typedef struct eze {
    SemVerX* local;   // Local resolution
    SemVerX* remote;  // Remote resolution
    SemVerX* archive; // Archive resolution
    int consensus;    // 2+ for resolved
} eze;

// Uche (knowledge) pointer - points to row-col matrix for coherence
typedef int** uche;  // 2D matrix [row][col] for topology

// Obi (heart/soul) pointer - for SemVer resolution
typedef SemVerX* obi;

// Iota (shared power) - shared polarity coherence
typedef Polarity iota;

// Tomography resolution function
int rift_tomography_resolve(eze* leader, uche knowledge, obi heart, iota shared, int rows, int cols);

// Polyglot encoding (like rift_open)
size_t rift_poly_encode(const uint8_t* in, size_t len, uint8_t* out, Polarity pol);

#endif