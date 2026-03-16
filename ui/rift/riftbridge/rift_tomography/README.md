# How to Use RIFT Tomography**

Below is the comprehensive `README.md` you requested.

```markdown
# RIFT Tomography - Trident Topology Resolver
*"Semantic Versioning with Fault-Tolerant Dependency Resolution"*

## 🌟 Overview
RIFT Tomography implements **trident topology** for dependency resolution, combining:
1. **R_open's 2→1 sparse duplex encoding** for package integrity
2. **SemVerX's stateful versioning** (major.minor.patch.channel)
3. **Tri-node consensus** (Upload/Runtime/Backup) for fault isolation

## 🏗️ Architecture

### Core Concepts
| Concept | Implementation | Purpose |
|---------|---------------|---------|
| **Trident Topology** | 3-node BiDAG (X→Y←Z) | Diamond problem prevention |
| **Eze Power** | `eze_power` in `ObiSoulPointer` | Authority level for resolution |
| **Uche Knowledge** | `uche_soul` pointer | Context-aware decision making |
| **Sparse Encoding** | `rift_encode_duplex()` | Package integrity verification |

### Resolution Flow
```
Package Spec → [Encoder] → Trident Nodes → Consensus Check → Bound/Unbound
    ↑                                    ↓
    └─── A* Scoring ←─ Eulerian Cycle ←──┘
```

## 🚀 Quick Start

### Prerequisites
- GCC/Clang (C/C++)
- .NET SDK (C#)
- CMake 3.15+

### Build Everything
```bash
# Clone and build
git clone https://github.com/your-org/RIFTTomography
cd RIFTTomography/build
chmod +x build.sh
./build.sh  # Builds C, C++, C# components
```

### Usage Examples

**1. C CLI (Direct Core Access)**
```bash
./bin/c/RIFTTomography resolve --package package.json --strategy hybrid
# Output: BOUND (consensus achieved) or UNBOUND_FAULT (isolated)
```

**2. C++ (Object-Oriented Wrapper)**
```cpp
#include <RIFTTomography/TridentResolver.hpp>

TridentResolver resolver("my-package@1.0.0-stable");
auto result = resolver.resolveWithAStar();
if (result.status == STATUS_BOUND) {
    std::cout << "✅ Safe to deploy" << std::endl;
}
```

**3. C# (.NET Integration)**
```csharp
using RiftTomography;

var resolver = new TridentResolver();
var context = new ObiSoulContext(ezePower: 5); // High authority
var result = resolver.Resolve("Newtonsoft.Json@13.0.1-stable", context);
Console.WriteLine($"Resolution: {result.Status}");
```

**4. Shell/Polyglot Encoding**
```bash
# Use R_open encoding for package verification
./bin/c/rift_encode --file package.tar.gz --polarity A
# Output: 2→1 encoded stream for integrity check
```

## 🔧 Advanced Configuration

### Trident Node Roles
Configure in `trident_config.json`:
```json
{
  "nodes": [
    {"role": "upload", "endpoint": "https://registry.local/upload"},
    {"role": "runtime", "endpoint": "https://runtime.local"},
    {"role": "backup", "endpoint": "https://backup.local/archive"}
  ],
  "consensus_threshold": 2,
  "encoding_polarity": "A"
}
```

### Resolution Strategies
| Strategy | Use Case | Complexity |
|----------|----------|------------|
| `Eulerian` | Fast dependency graph traversal | O(E) |
| `Hamiltonian` | Complete component verification | O(N²) |
| `AStar` | Optimal path with scoring | O(b^d) |
| `Hybrid` | Default: Eulerian + A* fallback | O(E log V) |

## 🧪 Testing
```bash
# Run test suite
cd build && ./test_all.sh

# Test specific components
./test_c.sh    # C core tests
./test_cpp.sh  # C++ wrapper tests
dotnet test src/csharp/RiftTomography.Tests/
```

## 📊 Performance
- **Resolution Time**: O(n log n) with trident pruning
- **Memory**: ½ log n auxiliary space (from R_open AVL)
- **Fault Containment**: Immediate isolation on version mismatch
- **Hot-Swap**: Zero-downtime component replacement

## 🔌 Integration

### With Existing Package Managers
```bash
# npm/yarn wrapper
npx rift-tompgy wrap-npm install lodash

# pip integration
python -m RIFTTomography.wrap_pip install requests==2.28.1
```

### CI/CD Pipeline
```yaml
# .github/workflows/resolve.yml
- name: Trident Resolution
  uses: rift-tompgy/resolve-action@v1
  with:
    strategy: 'hybrid'
    eze_power: 7
    fail_on_unbound: true
```

## 🐛 Troubleshooting

| Issue | Solution |
|-------|----------|
| "UNBOUND_FAULT" | Check version consistency across X,Y,Z nodes |
| Encoding mismatch | Ensure same polarity (A/B) across all nodes |
| Memory leak | Use `trident_cleanup()` after resolution |
| C# P/Invoke errors | Verify native library path in `RiftNative.cs` |

## 📚 Further Reading
- [R_open Sparse Encoding](https://github.com/obinexus/R_open)
- [SemVerX Trident Paper](https://obinexus.medium.com/semverx-trident-topology)
- [BiDAG Mathematics](https://en.wikipedia.org/wiki/Directed_acyclic_graph)

## 📄 License
OBINexus NT License v1.0 - See [LICENSE](LICENSE)

---
*"Good behavior shall be rewarded through systematic progression"* - OBINexus Constitution
```