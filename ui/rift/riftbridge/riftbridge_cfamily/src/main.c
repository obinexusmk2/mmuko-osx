#include <rift/riftbridge.h>
#include <rift/rift.h>
#include <rift/riftest.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
    // SemVerX examples
    SemVerX local = {1, 0, 0, CHANNEL_STABLE, POLARITY_POS};
    SemVerX remote = {1, 0, 0, CHANNEL_STABLE, POLARITY_POS};
    SemVerX archive = {1, 0, 0, CHANNEL_STABLE, POLARITY_POS};

    // Trident eze
    eze* leader = riftbridge_trident_init(&local, &remote, &archive);

    // Matrix for coherence (uche)
    uche matrix = (uche)malloc(2 * sizeof(int*));
    for (int i = 0; i < 2; i++) {
        matrix[i] = (int*)malloc(2 * sizeof(int));
        matrix[i][0] = POLARITY_POS;
        matrix[i][1] = POLARITY_POS;
    }

    // Test
    riftest_tomography(leader, matrix, 2, 2);
    riftest_poly_encode("test input");

    free(leader);
    for (int i = 0; i < 2; i++) free(matrix[i]);
    free(matrix);
    return 0;
}