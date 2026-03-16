# Rift Tomography

Rift Tomography is a C-family dependency resolution module for RIFT, extending ropen.c from github.com/obinexus/R_open. It uses tomography (radio-wave-like "seeing" of dependency topology surfaces) with trident SemVer resolution (local/remote/archive consensus) for polarity coherence in a row-col matrix.

## How Tomography Works via Resolution Trident
1. **Tomography Query**: "Sees" dependency structures as a row-col matrix (rows: flow, cols: depth). Uses radio metaphor to query surfaces without mutation.
2. **Trident Resolution**: 3 pointers (local, remote, archive) form trident. Consensus if ≥2 SemVerX match (major.minor.patch.channel.polarity).
3. **Polarity Coherence**: Positive (+) or negative (-) states checked in matrix via iota (shared power).
4. **Pointers**:
   - Eze (leader): Holds trident struct.
   - Iota (shared): Polarity value.
   - Uche (knowledge): Matrix pointer.
   - Obi (heart): Resolved SemVerX pointer.
5. **Encoding**: Polyglot like rift_open (2->1 duplex with polarity prune).

## Build & Usage
See build.sh and folder steps above.

## License
OBINexus NT License (as provided).