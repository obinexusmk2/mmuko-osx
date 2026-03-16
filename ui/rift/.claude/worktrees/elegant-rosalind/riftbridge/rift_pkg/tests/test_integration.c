#include "rift/riftest.h"
#include <stdio.h>
#include <stdlib.h>

// Test trident topology creation
bool test_trident_creation() {
    TridentNode* node = trident_create_node("test-package", "1.0.0-stable");
    RIFT_TEST_ASSERT(node != NULL);
    RIFT_TEST_ASSERT_STR_EQUAL("test-package", node->package_name);
    RIFT_TEST_ASSERT(node->version.major == 1);
    RIFT_TEST_ASSERT(node->version.minor == 0);
    RIFT_TEST_ASSERT(node->version.patch == 0);
    RIFT_TEST_ASSERT(node->version.channel == CHANNEL_STABLE);
    
    free(node->package_name);
    free(node);
    return true;
}

// Test SemVerX parsing
bool test_semverx_parsing() {
    SemVerX version = semverx_parse("2.1.3-experimental");
    RIFT_TEST_ASSERT(version.major == 2);
    RIFT_TEST_ASSERT(version.minor == 1);
    RIFT_TEST_ASSERT(version.patch == 3);
    RIFT_TEST_ASSERT(version.channel == CHANNEL_EXPERIMENTAL);
    
    char* formatted = semverx_format(version);
    RIFT_TEST_ASSERT(strstr(formatted, "2.1.3") != NULL);
    RIFT_TEST_ASSERT(strstr(formatted, "experimental") != NULL);
    
    return true;
}

// Test version constraints
bool test_version_constraints() {
    SemVerX version = semverx_parse("1.2.3-stable");
    
    // Test caret constraint
    RIFT_TEST_ASSERT(semverx_satisfies(version, "^1.0.0"));
    RIFT_TEST_ASSERT(!semverx_satisfies(version, "^2.0.0"));
    
    // Test tilde constraint
    RIFT_TEST_ASSERT(semverx_satisfies(version, "~1.2.0"));
    RIFT_TEST_ASSERT(!semverx_satisfies(version, "~1.3.0"));
    
    // Test exact constraint
    RIFT_TEST_ASSERT(semverx_satisfies(version, "=1.2.3-stable"));
    RIFT_TEST_ASSERT(!semverx_satisfies(version, "=1.2.4-stable"));
    
    return true;
}

// Test trident binding
bool test_trident_binding() {
    TridentNode* node1 = trident_create_node("dep1", "1.0.0-stable");
    TridentNode* node2 = trident_create_node("dep2", "1.0.0-stable");
    TridentNode* app = trident_create_node("app", "1.0.0-stable");
    
    // Set up identical incoming nodes
    app->incoming[0] = node1;
    app->incoming[1] = node2;
    
    // Should bind successfully
    RIFT_TEST_ASSERT(trident_bind_node(app));
    RIFT_TEST_ASSERT(app->is_bound);
    RIFT_TEST_ASSERT(app->confidence > 0.0f);
    
    // Test with different versions
    node2->version.minor = 1;  // Make versions different
    RIFT_TEST_ASSERT(!trident_bind_node(app));
    RIFT_TEST_ASSERT(!app->is_bound);
    
    free(node1->package_name);
    free(node2->package_name);
    free(app->package_name);
    free(node1);
    free(node2);
    free(app);
    
    return true;
}

// Test package installation
bool test_package_installation() {
    RiftBridgeContext* ctx = riftbridge_init("test-workspace");
    RIFT_TEST_ASSERT(ctx != NULL);
    
    // Test package query
    PackageEntry* entry = riftbridge_query(ctx, "test-package");
    // Should return NULL since package doesn't exist
    RIFT_TEST_ASSERT(entry == NULL);
    
    riftbridge_cleanup(ctx);
    return true;
}

// Test RIFT bridge initialization
bool test_bridge_initialization() {
    RiftBridgeContext* ctx = riftbridge_init("test-workspace");
    RIFT_TEST_ASSERT(ctx != NULL);
    RIFT_TEST_ASSERT(ctx->workspace_path != NULL);
    RIFT_TEST_ASSERT(ctx->cache_path != NULL);
    
    riftbridge_cleanup(ctx);
    return true;
}

// Main test runner
int main() {
    printf("Running RIFT Package Manager Integration Tests\n");
    printf("================================================\n\n");
    
    RiftTestSuite* suite = riftest_create_suite("RIFT Integration Tests");
    
    // Add tests
    riftest_add_test(suite, "Trident Creation", test_trident_creation, "Test creating trident nodes");
    riftest_add_test(suite, "SemVerX Parsing", test_semverx_parsing, "Test SemVerX version parsing");
    riftest_add_test(suite, "Version Constraints", test_version_constraints, "Test version constraint matching");
    riftest_add_test(suite, "Trident Binding", test_trident_binding, "Test trident node binding");
    riftest_add_test(suite, "Package Installation", test_package_installation, "Test package installation");
    riftest_add_test(suite, "Bridge Initialization", test_bridge_initialization, "Test RIFT bridge initialization");
    
    // Run tests
    bool success = riftest_run_suite(suite);
    
    // Print results
    riftest_print_results(suite);
    
    // Cleanup
    riftest_destroy_suite(suite);
    
    printf("\nTest run completed. %s\n", success ? "All tests passed!" : "Some tests failed.");
    return success ? 0 : 1;
}