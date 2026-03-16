#include "rift/riftbridge.h"
#include "rift/rift.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Print usage information
void print_usage(const char* program_name) {
    printf("RIFT Package Manager - C Implementation\n");
    printf("Usage: %s <command> [options]\n\n", program_name);
    printf("Commands:\n");
    printf("  install <package> [version]  Install a package\n");
    printf("  remove <package>              Remove a package\n");
    printf("  update <package>              Update a package\n");
    printf("  query <package>               Query package information\n");
    printf("  list                          List installed packages\n");
    printf("  sync                          Sync with remote registry\n");
    printf("  help                          Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s install semverx ^1.0.0\n", program_name);
    printf("  %s install trident latest\n", program_name);
    printf("  %s remove old-package\n", program_name);
    printf("  %s update all\n", program_name);
}

// Handle install command
int handle_install(RiftBridgeContext* ctx, int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Error: Package name required\n");
        return 1;
    }
    
    const char* package_name = argv[2];
    const char* version_spec = argc > 3 ? argv[3] : NULL;
    
    return riftbridge_install(ctx, package_name, version_spec);
}

// Handle remove command
int handle_remove(RiftBridgeContext* ctx, int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Error: Package name required\n");
        return 1;
    }
    
    const char* package_name = argv[2];
    return riftbridge_remove(ctx, package_name);
}

// Handle update command
int handle_update(RiftBridgeContext* ctx, int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Error: Package name required\n");
        return 1;
    }
    
    const char* package_name = argv[2];
    return riftbridge_update(ctx, package_name);
}

// Handle query command
int handle_query(RiftBridgeContext* ctx, int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Error: Package name required\n");
        return 1;
    }
    
    const char* package_name = argv[2];
    PackageEntry* entry = riftbridge_query(ctx, package_name);
    
    if (entry) {
        printf("Package: %s\n", entry->name);
        printf("Version: %s\n", entry->version_string);
        printf("Hash: %s\n", entry->tarball_hash);
        printf("Signature: %s\n", entry->signature);
        printf("Timestamp: %lu\n", entry->timestamp);
        return 0;
    } else {
        fprintf(stderr, "Package %s not found\n", package_name);
        return 1;
    }
}

// Handle list command
int handle_list(RiftBridgeContext* ctx, int argc, char* argv[]) {
    printf("Installed packages:\n");
    
    // This would iterate through the trident topology
    // For now, just show a message
    printf("No packages installed (topology empty)\n");
    return 0;
}

// Handle sync command
int handle_sync(RiftBridgeContext* ctx, int argc, char* argv[]) {
    const char* remote_url = argc > 2 ? argv[2] : "https://registry.obinexus.com";
    
    printf("Syncing with remote registry: %s\n", remote_url);
    int result = riftbridge_sync_remote(ctx, remote_url);
    
    if (result == 0) {
        printf("Sync completed successfully\n");
    } else {
        fprintf(stderr, "Sync failed\n");
    }
    
    return result;
}

// Main function
int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char* command = argv[1];
    
    // Initialize RIFT bridge context
    RiftBridgeContext* ctx = riftbridge_init(".");
    if (!ctx) {
        fprintf(stderr, "Error: Failed to initialize RIFT bridge\n");
        return 1;
    }
    
    int result = 0;
    
    // Handle commands
    if (strcmp(command, "install") == 0) {
        result = handle_install(ctx, argc, argv);
    } else if (strcmp(command, "remove") == 0) {
        result = handle_remove(ctx, argc, argv);
    } else if (strcmp(command, "update") == 0) {
        result = handle_update(ctx, argc, argv);
    } else if (strcmp(command, "query") == 0) {
        result = handle_query(ctx, argc, argv);
    } else if (strcmp(command, "list") == 0) {
        result = handle_list(ctx, argc, argv);
    } else if (strcmp(command, "sync") == 0) {
        result = handle_sync(ctx, argc, argv);
    } else if (strcmp(command, "help") == 0 || strcmp(command, "--help") == 0) {
        print_usage(argv[0]);
        result = 0;
    } else {
        fprintf(stderr, "Error: Unknown command '%s'\n", command);
        print_usage(argv[0]);
        result = 1;
    }
    
    // Cleanup
    riftbridge_cleanup(ctx);
    return result;
}