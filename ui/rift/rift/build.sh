#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

OUT_DIR="$SCRIPT_DIR/../../../artifacts/c/rift-pipeline"
mkdir -p "$OUT_DIR"

gcc -c -I. -Iinclude nsigii-codec/nsigii_codec.c -o "$OUT_DIR/nsigii_codec.o" -lm
gcc -c -I. -Iinclude rift-000/rift_000_tokenizer.c -o "$OUT_DIR/rift_000.o"
gcc -c -I. -Iinclude rift-001/rift_001_process.c -o "$OUT_DIR/rift_001.o"
gcc -c -I. -Iinclude rift-333/rift_333_ast.c -o "$OUT_DIR/rift_333.o"
gcc -c -I. -Iinclude rift-444/rift_444_target.c -o "$OUT_DIR/rift_444.o"
gcc -c -I. -Iinclude rift-555/rift_555_bridge.c -o "$OUT_DIR/rift_555.o"
gcc -c -I. -Iinclude main.c -o "$OUT_DIR/main.o"

gcc "$OUT_DIR"/*.o -o "$OUT_DIR/rift" -lm -lpthread
