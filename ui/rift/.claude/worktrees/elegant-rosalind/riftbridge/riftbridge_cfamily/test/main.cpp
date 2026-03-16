#include <rift/riftbridge.h>
#include <rift/rift.h>
#include <rift/riftest.h>
#include <iostream>

int main() {
    // SemVerX
    SemVerX local{1, 0, 0, CHANNEL_STABLE, POLARITY_POS};
    SemVerX remote{1, 0, 0, CHANNEL_STABLE, POLARITY_POS};
    SemVerX archive{1, 0, 0, CHANNEL_STABLE, POLARITY_POS};

    // Trident
    eze* leader = riftbridge_trident_init(&local, &remote, &archive);

    // Matrix (uche)
    uche matrix = new int*[2];
    for (int i = 0; i < 2; ++i) {
        matrix[i] = new int[2]{POLARITY_POS, POLARITY_POS};
    }

    // Test
    riftest_tomography(leader, matrix, 2, 2);
    riftest_poly_encode("test input");

    delete leader;
    for (int i = 0; i < 2; ++i) delete[] matrix[i];
    delete[] matrix;
    return 0;
}