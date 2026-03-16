#ifndef RIFTEST_H
#define RIFTEST_H

#include "rift.h"

// Test tomography resolution
void riftest_tomography(eze* leader, uche matrix, int rows, int cols);

// Test poly encoding
void riftest_poly_encode(const char* input);

#endif