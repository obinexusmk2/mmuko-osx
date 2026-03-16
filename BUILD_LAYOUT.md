# Build Layout and Canonical Commands

This repository uses one orchestrator per subtree and isolates outputs away from source directories.

## Orchestrator ownership

- **Root (`src/` + `include/`)**: GNU Make (`Makefile` at repository root).
- **RIFT subtree (`ui/rift/rift/`)**: CMake only.
- **C# UI (`ui/rift.csproj`)**: `dotnet` project build.

## Canonical language builds

| Language | Canonical command | Output location |
|---|---|---|
| C (root native) | `make build-c` | `build/` |
| C++ (root native bridge) | `make build-cpp` | `build/` |
| C# (.NET UI) | `make build-csharp` | `artifacts/csharp/` |
| All languages | `make build-all` | `build/` + `artifacts/csharp/` |

## RIFT CMake build (subtree-owned)

Run CMake from outside the source tree:

```bash
cmake -S ui/rift/rift -B ui/rift/rift/build
cmake --build ui/rift/rift/build
```

All generated files for this subtree must stay under `ui/rift/rift/build/`.

## CI guard

Use this script to assert outputs are not emitted into source folders:

```bash
ci/check-build-layout
```
