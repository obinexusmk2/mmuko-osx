#include <rift_tomogrpahy/trident.h>

typedef enum { UPLOAD_X, RUNTIME_Y, BACKUP_Z } NodeRole;

struct TridentNode {
    NodeRole role;
    SemVerX version;
    TridentNode* edges[2]; // Exactly 2 incoming edges for consensus
    ResolutionStatus status;
};

//! Consensus rule: node binds IFF both incoming versions are identical
ResolutionStatus trident_consensus_check(TridentNode* node) {
    if (!node->edges[0] || !node->edges[1]) return STATUS_UNBOUND;
    
    if (semverx_compare(&node->edges[0]->version, 
                       &node->edges[1]->version) == 0) {
        return STATUS_BOUND;
    }
    // Fault isolation: prevents inconsistent versions propagating
    return STATUS_UNBOUND_FAULT;
}