rift_pkg/
├── include/
│   ├── rift/
│   │   ├── riftbridge.h
│   │   ├── rift.h
│   │   └── riftest.h
│   └── semverx/
│       ├── semverx.h
│       ├── trident.h
│       └── registry.h
├── src/
│   ├── core/
│   │   ├── main.c
│   │   ├── main.cpp
│   │   └── main.cs
│   ├── trident/
│   │   ├── trident.c
│   │   ├── resolver.c
│   │   └── topology.c
│   ├── registry/
│   │   ├── local.c
│   │   ├── remote.c
│   │   └── avl_tree.c
│   └── bridge/
│       ├── riftbridge.c
│       ├── cpp_bridge.cpp
│       └── csharp_bridge.cs
├── build/
│   ├── Makefile
│   ├── build.sh
│   └── install.sh
├── tests/
│   ├── test_trident.c
│   ├── test_registry.c
│   └── test_integration.c
└── README.md