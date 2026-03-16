#include "rift/riftbridge.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Create a new trident node
TridentNode* trident_create_node(const char* package_name, const char* version_str) {
    TridentNode* node = calloc(1, sizeof(TridentNode));
    if (!node) return NULL;
    
    node->package_name = strdup(package_name);
    node->version = semverx_parse(version_str);
    node->incoming[0] = node->incoming[1] = NULL;
    node->outgoing[0] = NULL;
    node->is_bound = false;
    node->confidence = 1.0f;
    node->polarity = '+';  // Default positive polarity
    
    return node;
}

// Attempt to bind a trident node based on incoming versions
bool trident_bind_node(TridentNode* node) {
    if (!node) return false;
    
    // Consensus rule: both incoming versions must be identical
    if (node->incoming[0] && node->incoming[1]) {
        // Check if both incoming nodes have the same version
        if (node->incoming[0]->version.major == node->incoming[1]->version.major &&
            node->incoming[0]->version.minor == node->incoming[1]->version.minor &&
            node->incoming[0]->version.patch == node->incoming[1]->version.patch &&
            node->incoming[0]->version.channel == node->incoming[1]->version.channel) {
            
            node->is_bound = true;
            node->confidence = (node->incoming[0]->confidence + node->incoming[1]->confidence) / 2.0f;
            return true;
        }
    }
    
    // Single incoming node also binds (no conflict)
    if ((node->incoming[0] && !node->incoming[1]) || 
        (!node->incoming[0] && node->incoming[1])) {
        node->is_bound = true;
        node->confidence = node->incoming[0] ? node->incoming[0]->confidence : node->incoming[1]->confidence;
        return true;
    }
    
    node->is_bound = false;
    node->confidence = 0.0f;
    return false;
}

// Resolve diamond dependency problem
bool trident_resolve_diamond(TridentNode* app_node, const char* dependency_name) {
    if (!app_node || !dependency_name) return false;
    
    // Find all nodes with the same dependency name in the topology
    TridentNode** candidates = NULL;
    size_t candidate_count = 0;
    
    // Simple resolution: find the highest confidence version
    TridentNode* best_candidate = NULL;
    float max_confidence = 0.0f;
    
    // In a real implementation, this would traverse the entire topology
    // For now, we'll use a simplified approach
    
    if (app_node->incoming[0] && strcmp(app_node->incoming[0]->package_name, dependency_name) == 0) {
        if (app_node->incoming[0]->confidence > max_confidence) {
            max_confidence = app_node->incoming[0]->confidence;
            best_candidate = app_node->incoming[0];
        }
    }
    
    if (app_node->incoming[1] && strcmp(app_node->incoming[1]->package_name, dependency_name) == 0) {
        if (app_node->incoming[1]->confidence > max_confidence) {
            max_confidence = app_node->incoming[1]->confidence;
            best_candidate = app_node->incoming[1];
        }
    }
    
    if (best_candidate) {
        // Bind to the best candidate
        app_node->incoming[0] = best_candidate;
        app_node->incoming[1] = NULL;  // Clear conflicting dependency
        return trident_bind_node(app_node);
    }
    
    return false;
}

// Prune nodes with negative polarity or low confidence
void trident_prune_negative(TridentNode* topology[], size_t size) {
    if (!topology || size == 0) return;
    
    const float PRUNE_THRESHOLD = 0.5f;
    
    for (size_t i = 0; i < size; i++) {
        TridentNode* node = topology[i];
        if (!node) continue;
        
        // Prune negative polarity or low confidence
        if (node->polarity == '-' || node->confidence < PRUNE_THRESHOLD) {
            node->is_bound = false;
            node->confidence = 0.0f;
            
            // Disconnect from topology
            if (node->incoming[0]) node->incoming[0] = NULL;
            if (node->incoming[1]) node->incoming[1] = NULL;
            if (node->outgoing[0]) node->outgoing[0] = NULL;
        }
    }
}

// Build trident topology from package dependencies
TridentNode** trident_build_topology(PackageEntry* packages, size_t package_count, size_t* topology_size) {
    if (!packages || package_count == 0) return NULL;
    
    TridentNode** topology = calloc(package_count, sizeof(TridentNode*));
    if (!topology) return NULL;
    
    // Create nodes for each package
    for (size_t i = 0; i < package_count; i++) {
        topology[i] = trident_create_node(packages[i].name, packages[i].version_string);
        if (topology[i]) {
            topology[i]->confidence = 1.0f;  // Initial confidence
        }
    }
    
    *topology_size = package_count;
    return topology;
}