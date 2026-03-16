# Building MMUKO-OS

## Generated File Policy

Generated files must never be committed to git. Build outputs are written only to canonical artifact directories:

- `artifacts/c/`
- `artifacts/cpp/`
- `artifacts/csharp/`

Do not place compiled binaries, object files, CMake outputs, or .NET `obj/bin` files in source folders.

## Build Commands

- Root C/C++ build:
  - `make`
  - `make ringboot`
  - `make riftbridge`
  - `make bootimg`
- C# build:
  - `make csharp`
- RIFTLang build:
  - `make -C ui/riftlang`
- RIFT pipeline script build:
  - `./ui/rift/rift/build.sh`

All commands above now emit artifacts under `artifacts/` only.
