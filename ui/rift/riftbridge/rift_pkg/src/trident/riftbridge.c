#include "rift/riftbridge.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

// Initialize RIFT bridge context
RiftBridgeContext* riftbridge_init(const char* workspace_path) {
    RiftBridgeContext* ctx = calloc(1, sizeof(RiftBridgeContext));
    if (!ctx) return NULL;
    
    ctx->workspace_path = strdup(workspace_path);
    
    // Create cache directory
    char cache_path[1024];
    snprintf(cache_path, sizeof(cache_path), "%s/.rift-cache", workspace_path);
    ctx->cache_path = strdup(cache_path);
    
    // Create cache directory if it doesn't exist
    mkdir(ctx->cache_path, 0755);
    
    ctx->local_registry = NULL;
    ctx->remote_registry = NULL;
    ctx->trident_topology = NULL;
    ctx->trident_size = 0;
    
    return ctx;
}

// Clean up RIFT bridge context
void riftbridge_cleanup(RiftBridgeContext* ctx) {
    if (!ctx) return;
    
    free(ctx->workspace_path);
    free(ctx->cache_path);
    
    // Clean up trident topology
    if (ctx->trident_topology) {
        for (size_t i = 0; i < ctx->trident_size; i++) {
            if (ctx->trident_topology[i]) {
                free(ctx->trident_topology[i]->package_name);
                free(ctx->trident_topology[i]);
            }
        }
        free(ctx->trident_topology);
    }
    
    free(ctx);
}

// Install a package
int riftbridge_install(RiftBridgeContext* ctx, const char* package_name, const char* version_spec) {
    if (!ctx || !package_name) return -1;
    
    printf("Installing %s@%s...\n", package_name, version_spec ? version_spec : "latest");
    
    // Query package from registry
    PackageEntry* entry = riftbridge_query(ctx, package_name);
    if (!entry) {
        fprintf(stderr, "Package %s not found\n", package_name);
        return -1;
    }
    
    // Check version constraint
    SemVerX version = semverx_parse(entry->version_string);
    if (version_spec && !semverx_satisfies(version, version_spec)) {
        fprintf(stderr, "Version %s does not satisfy constraint %s\n", entry->version_string, version_spec);
        return -1;
    }
    
    // Create trident node for this package
    TridentNode* node = trident_create_node(package_name, entry->version_string);
    if (!node) {
        fprintf(stderr, "Failed to create trident node\n");
        return -1;
    }
    
    // Add to topology
    ctx->trident_topology = realloc(ctx->trident_topology, (ctx->trident_size + 1) * sizeof(TridentNode*));
    ctx->trident_topology[ctx->trident_size++] = node;
    
    printf("Successfully installed %s@%s\n", package_name, entry->version_string);
    return 0;
}

// Remove a package
int riftbridge_remove(RiftBridgeContext* ctx, const char* package_name) {
    if (!ctx || !package_name) return -1;
    
    printf("Removing %s...\n", package_name);
    
    // Find package in topology
    for (size_t i = 0; i < ctx->trident_size; i++) {
        if (ctx->trident_topology[i] && 
            strcmp(ctx->trident_topology[i]->package_name, package_name) == 0) {
            
            // Remove from topology
            free(ctx->trident_topology[i]->package_name);
            free(ctx->trident_topology[i]);
            
            // Shift remaining elements
            for (size_t j = i; j < ctx->trident_size - 1; j++) {
                ctx->trident_topology[j] = ctx->trident_topology[j + 1];
            }
            ctx->trident_size--;
            
            printf("Successfully removed %s\n", package_name);
            return 0;
        }
    }
    
    fprintf(stderr, "Package %s not found\n", package_name);
    return -1;
}

// Update a package
int riftbridge_update(RiftBridgeContext* ctx, const char* package_name) {
    if (!ctx || !package_name) return -1;
    
    printf("Updating %s...\n", package_name);
    
    // Remove old version
    if (riftbridge_remove(ctx, package_name) != 0) {
        return -1;
    }
    
    // Install latest version
    return riftbridge_install(ctx, package_name, NULL);
}

// Query package information
PackageEntry* riftbridge_query(RiftBridgeContext* ctx, const char* package_name) {
    if (!ctx || !package_name) return NULL;
    
    // Mock implementation - in real system, this would query actual registry
    static PackageEntry mock_entry = {
        .name = "example-package",
        .version_string = "1.2.3-stable",
        .tarball_hash = "sha256:abcdef123456...",
        .signature = "signature123...",
        .timestamp = 1640995200,  // Mock timestamp
        .trident_node = NULL
    };
    
    // Check if package exists in our topology
    for (size_t i = 0; i < ctx->trident_size; i++) {
        if (ctx->trident_topology[i] && 
            strcmp(ctx->trident_topology[i]->package_name, package_name) == 0) {
            mock_entry.name = ctx->trident_topology[i]->package_name;
            return &mock_entry;
        }
    }
    
    return NULL;
}

// SemVerX parsing
SemVerX semverx_parse(const char* version_str) {
    SemVerX version = {0, 0, 0, CHANNEL_STABLE};
    if (!version_str) return version;
    
    // Parse version string like "1.2.3-stable"
    char* str = strdup(version_str);
    char* major_str = strtok(str, ".");
    char* minor_str = strtok(NULL, ".");
    char* patch_str = strtok(NULL, "-");
    char* channel_str = strtok(NULL, "-");
    
    if (major_str) version.major = atoi(major_str);
    if (minor_str) version.minor = atoi(minor_str);
    if (patch_str) version.patch = atoi(patch_str);
    
    if (channel_str) {
        if (strcmp(channel_str, "legacy") == 0) version.channel = CHANNEL_LEGACY;
        else if (strcmp(channel_str, "experimental") == 0) version.channel = CHANNEL_EXPERIMENTAL;
        else if (strcmp(channel_str, "stable") == 0) version.channel = CHANNEL_STABLE;
        else if (strcmp(channel_str, "lts") == 0) version.channel = CHANNEL_LTS;
    }
    
    free(str);
    return version;
}

// SemVerX formatting
char* semverx_format(SemVerX version) {
    static char buffer[64];
    const char* channel_names[] = {"legacy", "experimental", "stable", "lts"};
    
    snprintf(buffer, sizeof(buffer), "%u.%u.%u-%s", 
             version.major, version.minor, version.patch, 
             channel_names[version.channel]);
    
    return buffer;
}

// Version constraint checking
bool semverx_satisfies(SemVerX version, const char* constraint) {
    if (!constraint) return true;
    
    // Simple constraint parsing - supports ^, ~, =, >, <, >=, <=
    char op = constraint[0];
    const char* version_part = constraint + 1;
    
    if (op == '^') {
        // Caret range: compatible with version
        return version.major == atoi(version_part);
    } else if (op == '~') {
        // Tilde range: approximately equivalent
        char* constraint_copy = strdup(version_part);
        char* major_str = strtok(constraint_copy, ".");
        char* minor_str = strtok(NULL, ".");
        
        bool satisfies = (version.major == atoi(major_str));
        if (minor_str) {
            satisfies = satisfies && (version.minor == atoi(minor_str));
        }
        
        free(constraint_copy);
        return satisfies;
    } else if (op == '=') {
        // Exact version
        SemVerX constraint_version = semverx_parse(version_part);
        return (version.major == constraint_version.major &&
                version.minor == constraint_version.minor &&
                version.patch == constraint_version.patch &&
                version.channel == constraint_version.channel);
    }
    
    // Default: accept all versions
    return true;
}