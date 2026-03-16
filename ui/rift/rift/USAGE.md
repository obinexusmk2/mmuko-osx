# RIFT Pipeline Usage Guide

## Quick Start

### Building from Source

```bash
# Clone the repository
cd rift-pipeline

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make -j$(nproc)

# Run tests
make test
```

### Manual Compilation (without CMake)

```bash
cd rift-pipeline

# Compile all source files
gcc -c -I. -Iinclude nsigii-codec/nsigii_codec.c -o nsigii_codec.o -lm
gcc -c -I. -Iinclude rift-000/rift_000_tokenizer.c -o rift_000.o
gcc -c -I. -Iinclude rift-001/rift_001_process.c -o rift_001.o
gcc -c -I. -Iinclude rift-333/rift_333_ast.c -o rift_333.o
gcc -c -I. -Iinclude rift-444/rift_444_target.c -o rift_444.o
gcc -c -I. -Iinclude rift-555/rift_555_bridge.c -o rift_555.o
gcc -c -I. -Iinclude main.c -o main.o

# Link executable
gcc *.o -o rift -lm -lpthread
```

## Command Line Usage

### Basic Usage

```bash
# Run demo (no arguments)
./rift

# Process a RIFT source file
./rift input.rift

# Specify output file
./rift -o output.c input.rift

# Execute up to specific stage
./rift -s 333 input.rift    # Stop at AST
./rift -s 444 input.rift    # Full compilation

# Show version
./rift -v

# Show help
./rift -h
```

### Stage Selection

| Stage | Description | Output |
|-------|-------------|--------|
| 000 | Tokenization | Token stream |
| 001 | Process/Procedure | Filtered tokens with NSIGII encoding |
| 333 | AST Generation | Abstract Syntax Tree |
| 444 | Target Generation | C code for target platform |
| 555 | RIFTBridge | Polyglot output (C/C++/C#) |

## C API Usage

### Basic Pipeline Execution

```c
#include <rift_pipeline.h>
#include <rift_555_bridge.h>

int main() {
    // Create RIFTBridge
    RiftBridge* bridge = riftbridge_create(RIFT_POLAR_C);
    if (!bridge) {
        fprintf(stderr, "Failed to create bridge\n");
        return 1;
    }
    
    // Initialize
    if (riftbridge_initialize(bridge) != 0) {
        fprintf(stderr, "Failed to initialize: %s\n", bridge->error_message);
        riftbridge_destroy(bridge);
        return 1;
    }
    
    // Execute full pipeline
    const char* input = "int main() { return 42; }";
    if (riftbridge_execute_pipeline(bridge, input) != 0) {
        fprintf(stderr, "Pipeline failed: %s\n", bridge->error_message);
        riftbridge_destroy(bridge);
        return 1;
    }
    
    // Cleanup
    riftbridge_destroy(bridge);
    return 0;
}
```

### Stage-by-Stage Execution

```c
#include <rift_pipeline.h>

int main() {
    RiftPipeline* pipeline = rift_pipeline_create();
    if (!pipeline) return 1;
    
    const char* input = "int x = 42;";
    
    // Stage 000: Tokenize
    if (rift_stage_000_tokenize(pipeline, input) != 0) {
        printf("Tokenization failed: %s\n", pipeline->error_message);
        rift_pipeline_destroy(pipeline);
        return 1;
    }
    
    // Stage 001: Process
    if (rift_stage_001_process(pipeline) != 0) {
        printf("Processing failed: %s\n", pipeline->error_message);
        rift_pipeline_destroy(pipeline);
        return 1;
    }
    
    // Stage 333: Build AST
    if (rift_stage_333_ast(pipeline) != 0) {
        printf("AST build failed: %s\n", pipeline->error_message);
        rift_pipeline_destroy(pipeline);
        return 1;
    }
    
    // Stage 444: Generate target
    if (rift_stage_444_target(pipeline) != 0) {
        printf("Target generation failed: %s\n", pipeline->error_message);
        rift_pipeline_destroy(pipeline);
        return 1;
    }
    
    rift_pipeline_destroy(pipeline);
    return 0;
}
```

### NSIGII Codec Usage

```c
#include <nsigii_codec.h>

int main() {
    // Create codec
    NsigiiCodec* codec = nsigii_codec_create(64, 64);
    if (!codec) return 1;
    
    // Create RB-AVL tree
    NsigiiRBTree* tree = nsigii_rbtree_create();
    
    // Encode data using ROPEN sparse duplex
    uint8_t input[] = {0x12, 0x34, 0x56, 0x78};
    size_t output_len;
    uint8_t* output = nsigii_rift_encode(input, sizeof(input), true, tree, &output_len);
    
    printf("Encoded %zu bytes to %zu bytes\n", sizeof(input), output_len);
    
    // Cleanup
    free(output);
    nsigii_rbtree_destroy(tree);
    nsigii_codec_destroy(codec);
    
    return 0;
}
```

### Trident Pattern Matching

```c
#include <rift_555_bridge.h>

int main() {
    // Match hex pattern [A-Z][0-9]
    char arg_a = 'A';
    int arg_b = 1;
    
    bool match = rift_match_triplet(
        "[A-Z]", &arg_a,      // Pattern A: uppercase letter
        "[0-9]", &arg_b,      // Pattern B: digit
        RIFT_ANCHOR_END       // Anchor at end of lexeme
    );
    
    if (match) {
        printf("Pattern matched!\n");
    }
    
    return 0;
}
```

## C++ API Usage

### Using RIFTBridge C++ Wrapper

```cpp
#include <rift_555_bridge.hpp>
#include <iostream>

int main() {
    // Create bridge
    rift::Bridge bridge;
    if (!bridge.initialize()) {
        std::cerr << "Failed to initialize: " << bridge.error_message() << std::endl;
        return 1;
    }
    
    // Execute pipeline
    std::string input = R"(
        int main() {
            int x = 42;
            return x;
        }
    )";
    
    if (!bridge.execute_pipeline(input)) {
        std::cerr << "Pipeline failed: " << bridge.error_message() << std::endl;
        return 1;
    }
    
    std::cout << "Pipeline executed successfully!" << std::endl;
    return 0;
}
```

### Hex Pattern Matching with Templates

```cpp
#include <rift_555_bridge.hpp>
#include <iostream>

int main() {
    // Create hex matcher
    rift::HexMatcher<char, int> matcher;
    
    // Match pattern
    auto result = matcher.match("A1");
    
    std::cout << "Alpha: " << result.first << std::endl;
    std::cout << "Numeric: " << result.second << std::endl;
    
    return 0;
}
```

## RIFT Language Syntax

### Memory Span Declaration

```rift
align span<row> {
    direction: right -> left,
    bytes: 8^4,       // Million-bit scale memory vector
    type: continuous, // Span lives in active aligned memory
    open: true        // Span is mutable, appendable
}
```

### Variable Declaration

```rift
int x = 42;
int y = x + 10;
```

### Function Definition

```rift
int main() {
    int x = 42;
    return x;
}
```

### Token Triplet Model

All tokens in RIFT are represented as triplets:
- **Type**: Classification (INT, ROLE, MASK, OP, QBYTE, etc.)
- **Value**: Data or symbolic representation
- **Memory**: Memory alignment and governance policies

## NSIGII Codec Features

### Trident Channel Architecture

```
Channel 0 (Transmitter): 127.0.0.1 - ORDER state
Channel 1 (Receiver):    127.0.0.2 - CHAOS state
Channel 2 (Verifier):    127.0.0.3 - CONSENSUS state
```

### Discriminant Flash Verification

```c
NsigiiFilterFlash* flash = nsigii_filter_flash_create(1.0, 2.0, 1.0);
double delta = nsigii_filter_flash_compute_discriminant(flash);
NsigiiDiscriminantState state = nsigii_filter_flash_context_switch(flash);

switch (state) {
    case NSIGII_DISCRIMINANT_ORDER:     // Δ > 0
        printf("Stable encoding\n");
        break;
    case NSIGII_DISCRIMINANT_CONSENSUS: // Δ = 0
        printf("Flash point - consensus reached\n");
        break;
    case NSIGII_DISCRIMINANT_CHAOS:     // Δ < 0
        printf("Chaos state - requires repair\n");
        break;
}
```

### ROPEN Sparse Duplex Encoding

```c
// Two bytes → one logical byte
uint8_t a = 0x12;
uint8_t b = 0x34;
uint8_t logical = a ^ nsigii_conjugate(b);  // 0x12 ^ 0xCB = 0xD9
```

## Testing

### Run All Tests

```bash
./rift_tests
```

### Run Specific Test Category

```bash
./rift_tests basic      # Basic functionality
./rift_tests nsigii     # NSIGII codec tests
./rift_tests riftbridge # RIFTBridge tests
./rift_tests tokenizer  # Tokenizer tests
```

### Built-in Tests

```bash
./rift -t
```

## Troubleshooting

### Compilation Errors

**Error**: `undefined reference to 'regexec'`
**Solution**: Link with `-lregex` or `-lpcre` if using external regex library

**Error**: `undefined reference to 'pthread_create'`
**Solution**: Link with `-lpthread`

**Error**: `undefined reference to 'sqrt'`
**Solution**: Link with `-lm`

### Runtime Errors

**Error**: `Failed to initialize NSIGII codec`
**Solution**: Check memory availability and permissions

**Error**: `Stage 000 failed: Failed to allocate token buffer`
**Solution**: Increase available memory or reduce input size

## Performance Tuning

### NSIGII Compression Ratios

| Stage | Compression | Use Case |
|-------|-------------|----------|
| Raw RGB24 | 0% | Baseline |
| YUV420 | 50% | Video encoding |
| ROPEN Duplex | 75% | Token compression |
| DEFLATE | 93.75% | Final output |

### Memory Usage

- **RB-AVL Tree**: O(½ log n) space complexity
- **Token Buffer**: Configurable (default 1024 tokens)
- **AST Nodes**: Dynamically allocated per program

## Integration Examples

### Integrating with Existing C Projects

```c
// Add to your CMakeLists.txt
target_link_libraries(your_target rift_pipeline nsigii_codec)

// Include headers
#include <rift/rift_pipeline.h>
#include <rift/rift_555_bridge.h>
```

### Creating a Custom Stage

```c
#include <rift_pipeline.h>

int my_custom_stage(RiftPipeline* pipeline) {
    // Access tokens from previous stage
    TokenTriplet* tokens = pipeline->tokens;
    size_t count = pipeline->token_count;
    
    // Process tokens...
    
    // Update stage bound
    pipeline->bound.stage_id = 99;
    pipeline->current_stage = RIFT_STAGE_555;
    
    return 0;
}
```

## References

- [RIFT Token Architecture](../docs/token-architecture.md)
- [NSIGII Codec Specification](../docs/nsigii-spec.md)
- [Trident Anchor Pattern](../docs/trident-anchor.md)
- [Memory Governance](../docs/memory-governance.md)

---

*For more information, visit: github.com/obinexus/rift-pipeline*
