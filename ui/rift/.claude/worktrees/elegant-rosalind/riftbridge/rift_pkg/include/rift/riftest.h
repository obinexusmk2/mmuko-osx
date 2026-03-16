#ifndef RIFTEST_H
#define RIFTEST_H

#include "rift.h"
#include <stdbool.h>

// Test framework for RIFT
typedef struct {
    const char* name;
    bool (*test_func)(void);
    const char* description;
} RiftTest;

typedef struct {
    RiftTest* tests;
    size_t test_count;
    size_t passed_count;
    size_t failed_count;
    char* results_buffer;
    size_t buffer_size;
} RiftTestSuite;

// Test macros
#define RIFT_TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf("ASSERT FAILED: %s at %s:%d\n", #condition, __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define RIFT_TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf("ASSERT EQUAL FAILED: expected %ld, got %ld at %s:%d\n", \
                   (long)(expected), (long)(actual), __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define RIFT_TEST_ASSERT_STR_EQUAL(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            printf("ASSERT STR EQUAL FAILED: expected '%s', got '%s' at %s:%d\n", \
                   (expected), (actual), __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

// Test suite management
RiftTestSuite* riftest_create_suite(const char* suite_name);
void riftest_destroy_suite(RiftTestSuite* suite);
void riftest_add_test(RiftTestSuite* suite, const char* name, bool (*test_func)(void), const char* description);

// Test execution
bool riftest_run_suite(RiftTestSuite* suite);
bool riftest_run_test(RiftTestSuite* suite, const char* test_name);
void riftest_print_results(RiftTestSuite* suite);

// Built-in test helpers
bool riftest_test_trident_topology(void);
bool riftest_test_semverx_parsing(void);
bool riftest_test_package_resolution(void);
bool riftest_test_encoding_decoding(void);
bool riftest_test_cost_governance(void);

// Performance testing
typedef struct {
    double execution_time;
    size_t memory_usage;
    size_t operations_count;
    double throughput;
} RiftPerformanceMetrics;

RiftPerformanceMetrics* riftest_benchmark_test(bool (*test_func)(void), int iterations);
void riftest_print_metrics(RiftPerformanceMetrics* metrics);

#endif // RIFTEST_H