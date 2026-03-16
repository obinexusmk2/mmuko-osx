#include "riftbridge.h"
#include <stdlib.h>

// Init trident eze
eze* riftbridge_trident_init(SemVerX* local, SemVerX* remote, SemVerX* archive) {
    eze* leader = (eze*)malloc(sizeof(eze));
    leader->local = local;
    leader->remote = remote;
    leader->archive = archive;
    leader->consensus = 0;

    // Count agreements (simple strcmp on versions)
    if (memcmp(local, remote, sizeof(SemVerX)) == 0) leader->consensus++;
    if (memcmp(local, archive, sizeof(SemVerX)) == 0) leader->consensus++;
    if (memcmp(remote, archive, sizeof(SemVerX)) == 0) leader->consensus++;

    return leader;
}

// Consensus check
int riftbridge_consensus(eze* leader) {
    return leader->consensus >= 2 ? 1 : 0;
}

// Row-col coherence
int riftbridge_coherence(uche matrix, int rows, int cols, iota shared) {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if ((Polarity)matrix[r][c] != shared) return 0;
        }
    }
    return 1;
}