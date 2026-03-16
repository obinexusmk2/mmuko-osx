# RIFT Pipeline - RIFT Is a Flexible Translator

**Version**: 1.0.0  
**Author**: OBINexus Computing  
**Date**: February 2026  
**License**: MIT

## Overview

The RIFT Pipeline is a complete compiler infrastructure implementing the RIFT (RIFT Is a Flexible Translator) architecture. It provides a multi-stage pipeline from tokenization through to platform-specific code generation, with integrated NSIGII codec support for compression and verification.

### Pipeline Stages

```
Input (.rift)
    │
    ▼
[RIFT-000] Tokenization ────────► TokenTriplets (type, value, memory)
    │
    ▼
[RIFT-001] Process/Procedure ───► NSIGII LTE Codec, Validation
    │
    ▼
[RIFT-333] AST Target Program ──► Abstract Syntax Tree
    │
    ▼
[RIFT-444] Platform Target ─────► C Code (macOS/Windows/Linux)
    │
    ▼
[RIFT-555] RIFTBridge ──────────► Polyglot C/C++/C# Output
```

## Features

- **Token Triplet Model**: All tokens are (type, value, memory) triplets
- **NSIGII Codec Integration**: Trident channel architecture with discriminant flash verification
- **Polyglot Bridge**: Single codebase compiles to C, C++, and C#
- **Memory-First Parsing**: Memory alignment before type/value assignment
- **Stage-Bound Execution**: Deterministic pipeline progression
- **Zero-Config Target**: Minimal configuration required

## Quick Start

### Prerequisites

- CMake 3.10+
- C11/C++17 compatible compiler (GCC, Clang, MSVC)
- POSIX threads (optional, for thread safety)

### Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Run Tests

```bash
make test
```

### Run Demo

```bash
./rift
```

## Usage

### Command Line

```bash
# Run full pipeline on input file
./rift input.rift

# Execute up to specific stage
./rift -s 333 input.rift    # Stop at AST generation
./rift -s 444 input.rift    # Stop at target generation

# Use NSIGII codec for compression
./rift -n input.rift

# Write output to file
./rift -o output.c input.rift

# Run test suite
./rift -t
```

### C API

```c
#include <rift_pipeline.h>
#include <rift_555_bridge.h>

int main() {
    // Create bridge
    RiftBridge* bridge = riftbridge_create(RIFT_POLAR_C);
    riftbridge_initialize(bridge);
    
    // Execute full pipeline
    const char* input = "int main() { return 42; }";
    riftbridge_execute_pipeline(bridge, input);
    
    // Cleanup
    riftbridge_destroy(bridge);
    return 0;
}
```

### C++ API

```cpp
#include <rift_555_bridge.hpp>

int main() {
    // Create bridge
    rift::Bridge bridge;
    bridge.initialize();
    
    // Execute pipeline
    std::string input = "int main() { return 42; }";
    bridge.execute_pipeline(input);
    
    return 0;
}
```

## Architecture

### Stage 000: Tokenization

Converts raw input into TokenTriplets:
- `token_type`: Classification (INT, ROLE, MASK, OP, etc.)
- `token_value`: Data or symbolic representation
- `token_memory`: Memory alignment and governance policies

### Stage 001: Process/Procedure

Executes procedures with NSIGII LTE (Link-Then-Execute) codec:
- Token filtering
- Memory alignment
- NSIGII encoding
- Validation

### Stage 333: AST Target Program

Builds Abstract Syntax Tree:
- Function definitions
- Variable declarations
- Expressions and statements
- Control flow
- RIFT-specific constructs (spans, roles, masks)

### Stage 444: Platform Target

Generates platform-specific C code:
- macOS (Darwin)
- Windows (Win32/Win64)
- Linux

### Stage 555: RIFTBridge

Polyglot bridge providing unified interface across:
- C (procedural)
- C++ (templates, constexpr)
- C# (attributes, source generators)

## NSIGII Codec

The NSIGII (N-Sigma-II) Polygatic Video Codec provides:

### Trident Channel Architecture

- **Channel 0 (Transmitter)**: 127.0.0.1 - Encoding (ORDER)
- **Channel 1 (Receiver)**: 127.0.0.2 - Decoding (CHAOS)
- **Channel 2 (Verifier)**: 127.0.0.3 - Verification (CONSENSUS)

### Discriminant Flash Verification

Uses quadratic formula Δ = b² - 4ac:
- **Δ > 0**: ORDER state (stable encoding)
- **Δ = 0**: CONSENSUS state (flash point)
- **Δ < 0**: CHAOS state (requires repair)

### ROPEN Sparse Duplex

Two physical bytes → one logical byte:
```
Polarity A: logical = a ⊕ conjugate(b)
Polarity B: logical = conjugate(a) ⊕ b
conjugate(x) = 0xF ⊕ x
```

### Flash Buffer Operations

- **Additive**: 1/2 + 1/2 = 1
- **Multiplicative**: 1/2 × 1/2 = 1/4

## File Structure

```
rift-pipeline/
├── include/
│   └── rift_pipeline.h          # Core pipeline API
├── rift-000/
│   └── rift_000_tokenizer.c     # Tokenization stage
├── rift-001/
│   └── rift_001_process.c       # Process/Procedure stage
├── rift-333/
│   └── rift_333_ast.c           # AST stage
├── rift-444/
│   └── rift_444_target.c        # Platform target stage
├── rift-555/
│   ├── rift_555_bridge.h        # Polyglot bridge header
│   ├── rift_555_bridge.c        # Bridge implementation
│   └── rift_555_bridge_cpp.cpp  # C++ wrapper
├── nsigii-codec/
│   ├── nsigii_codec.h           # NSIGII codec API
│   └── nsigii_codec.c           # NSIGII implementation
├── tests/
│   └── test_rift.c              # Test suite
├── main.c                       # Main entry point
└── CMakeLists.txt               # Build configuration
```

## API Reference

### RiftBridge Functions

| Function | Description |
|----------|-------------|
| `riftbridge_create(polar)` | Create bridge with specified polar (C/C++/C#) |
| `riftbridge_destroy(bridge)` | Destroy bridge and free resources |
| `riftbridge_initialize(bridge)` | Initialize bridge and NSIGII codec |
| `riftbridge_tokenize(bridge, input)` | Execute stage 000 |
| `riftbridge_process(bridge)` | Execute stage 001 |
| `riftbridge_build_ast(bridge)` | Execute stage 333 |
| `riftbridge_generate_target(bridge)` | Execute stage 444 |
| `riftbridge_execute_pipeline(bridge, input)` | Execute full pipeline |

### NSIGII Codec Functions

| Function | Description |
|----------|-------------|
| `nsigii_codec_create(width, height)` | Create codec context |
| `nsigii_codec_destroy(codec)` | Destroy codec |
| `nsigii_rift_encode(input, len, polarity, tree, out_len)` | ROPEN encode |
| `nsigii_flash_unite(buffer)` | 1/2 + 1/2 = 1 |
| `nsigii_flash_multiply(buffer)` | 1/2 × 1/2 = 1/4 |
| `nsigii_filter_flash_create(a, b, c)` | Create flash verifier |
| `nsigii_filter_flash_compute_discriminant(flash)` | Compute Δ |

## Trident Anchor Pattern

The Trident Anchor provides isomorphic pattern matching across C, C++, and C#:

### YAML Definition
```yaml
hex_pattern:
  signature: [alpha, numeric]
  regex: "[A-Z][0-9]"
  capture_groups:
    - name: args
      type: token_alpha
    - name: vargs
      type: token_numeric
  anchor: end_of_lexeme
```

### C Usage
```c
#define RIFT_HEX(args, vargs) \
    rift_match_triplet("[A-Z]", &(args), "[0-9]", &(vargs), RIFT_ANCHOR_END)
```

### C++ Usage
```cpp
template<typename T, typename U>
auto rift_hex(T& args, U& vargs) {
    return rift_match<token_alpha, token_numeric>(
        std::regex("[A-Z]"), args,
        std::regex("[0-9]"), vargs,
        rift_anchor::end
    );
}
```

### C# Usage
```csharp
[GeneratedRegex(@"[A-Z][0-9]$")]
public static partial Regex HexPattern();
```

## Testing

```bash
# Run all tests
./rift_tests

# Run specific test category
./rift_tests nsigii      # NSIGII codec tests
./rift_tests riftbridge  # RIFTBridge tests
./rift_tests tokenizer   # Tokenizer tests
./rift_tests basic       # Basic tests
```

## Performance

### NSIGII Compression Ratios

| Stage | Size | Reduction |
|-------|------|-----------|
| Raw RGB24 | 248,832 bytes | baseline |
| YUV420 | 124,416 bytes | 50% |
| ROPEN Duplex | 62,208 bytes | 75% |
| DEFLATE | ~15,552 bytes | 93.75% |

### Computational Complexity

- **RB-AVL Tree**: O(½ log n) space
- **Duplex Encoding**: O(¼ log n) per byte
- **Tokenization**: O(n) linear

## Contributing

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## References

- NSIGII BiPolar Sequence Theory
- ROPEN Sparse Duplex Specification
- Trident Command & Control Architecture
- RIFT Token Architecture and Memory Governance

## License

MIT License - See LICENSE file for details.

---

*"Structure is a signal. Polarity is a strategy. RIFT is the experiment."*  
*— OBINexus Computing, 2026*
