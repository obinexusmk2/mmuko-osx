#ifndef RIFTBRIDGE_H
#define RIFTBRIDGE_H

#include "rift.h"

// Bridge function for trident resolution
eze* riftbridge_trident_init(SemVerX* local, SemVerX* remote, SemVerX* archive);

// Check consensus (2/3 agreement)
int riftbridge_consensus(eze* leader);

// Row-col coherence check
int riftbridge_coherence(uche matrix, int rows, int cols, iota shared);

#endif