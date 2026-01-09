#include <iostream>
#include <cassert>
#include <string>
#include <sstream>
#include <vector>
#include <random>
#include <algorithm>
#include <climits>

// Include the BTree implementation
#include "btree.cpp"

int tests_passed = 0;
int tests_failed = 0;

#define TEST(name) void name()
#define RUN_TEST(name) do { \
    std::cout << "Running " << #name << "... "; \
    try { \
        name(); \
        std::cout << "PASSED" << std::endl; \
        tests_passed++; \
    } catch (const std::exception& e) { \
        std::cout << "FAILED: " << e.what() << std::endl; \
        tests_failed++; \
    } catch (...) { \
        std::cout << "FAILED: Unknown exception" << std::endl; \
        tests_failed++; \
    } \
} while(0)

#define ASSERT_TRUE(condition) do { \
    if (!(condition)) { \
        throw std::runtime_error("Assertion failed: " #condition); \
    } \
} while(0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))
#define ASSERT_EQ(a, b) ASSERT_TRUE((a) == (b))

// Test: Empty tree should be empty
TEST(test_empty_tree) {
    BTree<int> tree;
    ASSERT_TRUE(tree.empty());
    ASSERT_FALSE(tree.search(42));
}

// Test: Single insertion
TEST(test_single_insert) {
    BTree<int> tree;
    tree.insert(10);
    ASSERT_FALSE(tree.empty());
    ASSERT_TRUE(tree.search(10));
    ASSERT_FALSE(tree.search(5));
}

// Test: Multiple insertions
TEST(test_multiple_inserts) {
    BTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(5);

    ASSERT_TRUE(tree.search(10));
    ASSERT_TRUE(tree.search(20));
    ASSERT_TRUE(tree.search(5));
    ASSERT_FALSE(tree.search(15));
}

// Test: Insertion in sorted order
TEST(test_sorted_insert) {
    BTree<int> tree;
    for (int i = 1; i <= 10; i++) {
        tree.insert(i);
    }

    for (int i = 1; i <= 10; i++) {
        ASSERT_TRUE(tree.search(i));
    }
    ASSERT_FALSE(tree.search(0));
    ASSERT_FALSE(tree.search(11));
}

// Test: Insertion in reverse sorted order
TEST(test_reverse_sorted_insert) {
    BTree<int> tree;
    for (int i = 10; i >= 1; i--) {
        tree.insert(i);
    }

    for (int i = 1; i <= 10; i++) {
        ASSERT_TRUE(tree.search(i));
    }
}

// Test: Large number of insertions (forces splits)
TEST(test_many_inserts) {
    BTree<int> tree;
    for (int i = 0; i < 100; i++) {
        tree.insert(i);
    }

    for (int i = 0; i < 100; i++) {
        ASSERT_TRUE(tree.search(i));
    }
    ASSERT_FALSE(tree.search(100));
    ASSERT_FALSE(tree.search(-1));
}

// Test: Random order insertions
TEST(test_random_insert) {
    BTree<int> tree;
    std::vector<int> values = {50, 25, 75, 10, 30, 60, 80, 5, 15, 27, 35, 55, 65, 77, 90};

    for (int val : values) {
        tree.insert(val);
    }

    for (int val : values) {
        ASSERT_TRUE(tree.search(val));
    }
    ASSERT_FALSE(tree.search(100));
}

// Test: Duplicate values (behavior depends on implementation)
TEST(test_duplicate_insert) {
    BTree<int> tree;
    tree.insert(10);
    tree.insert(10);  // Insert duplicate

    ASSERT_TRUE(tree.search(10));
}

// Test: Negative values
TEST(test_negative_values) {
    BTree<int> tree;
    tree.insert(-10);
    tree.insert(-5);
    tree.insert(0);
    tree.insert(5);
    tree.insert(10);

    ASSERT_TRUE(tree.search(-10));
    ASSERT_TRUE(tree.search(-5));
    ASSERT_TRUE(tree.search(0));
    ASSERT_TRUE(tree.search(5));
    ASSERT_TRUE(tree.search(10));
}

// Test: String type
TEST(test_string_type) {
    BTree<std::string> tree;
    tree.insert("apple");
    tree.insert("banana");
    tree.insert("cherry");

    ASSERT_TRUE(tree.search("apple"));
    ASSERT_TRUE(tree.search("banana"));
    ASSERT_TRUE(tree.search("cherry"));
    ASSERT_FALSE(tree.search("date"));
}

// Test: Double type
TEST(test_double_type) {
    BTree<double> tree;
    tree.insert(3.14);
    tree.insert(2.71);
    tree.insert(1.41);

    ASSERT_TRUE(tree.search(3.14));
    ASSERT_TRUE(tree.search(2.71));
    ASSERT_TRUE(tree.search(1.41));
    ASSERT_FALSE(tree.search(1.73));
}

// Test: Higher order BTree (Order = 5)
TEST(test_order_5) {
    BTree<int, 5> tree;
    for (int i = 0; i < 50; i++) {
        tree.insert(i);
    }

    for (int i = 0; i < 50; i++) {
        ASSERT_TRUE(tree.search(i));
    }
}

// Test: Higher order BTree (Order = 10)
TEST(test_order_10) {
    BTree<int, 10> tree;
    for (int i = 0; i < 100; i++) {
        tree.insert(i);
    }

    for (int i = 0; i < 100; i++) {
        ASSERT_TRUE(tree.search(i));
    }
}

// Test: Search on empty tree
TEST(test_search_empty_tree) {
    BTree<int> tree;
    ASSERT_FALSE(tree.search(0));
    ASSERT_FALSE(tree.search(100));
    ASSERT_FALSE(tree.search(-100));
}

// Test: Boundary values
TEST(test_boundary_values) {
    BTree<int> tree;
    tree.insert(INT_MAX);
    tree.insert(INT_MIN);
    tree.insert(0);

    ASSERT_TRUE(tree.search(INT_MAX));
    ASSERT_TRUE(tree.search(INT_MIN));
    ASSERT_TRUE(tree.search(0));
}

// Test: Stress test with shuffled data
TEST(test_stress_shuffled) {
    BTree<int, 4> tree;
    std::vector<int> values;
    for (int i = 0; i < 1000; i++) {
        values.push_back(i);
    }

    // Shuffle values
    std::mt19937 rng(42);  // Fixed seed for reproducibility
    std::shuffle(values.begin(), values.end(), rng);

    for (int val : values) {
        tree.insert(val);
    }

    for (int i = 0; i < 1000; i++) {
        ASSERT_TRUE(tree.search(i));
    }
}

// Test: Traverse outputs in sorted order (capture stdout)
TEST(test_traverse_order) {
    BTree<int> tree;
    tree.insert(30);
    tree.insert(10);
    tree.insert(20);
    tree.insert(40);
    tree.insert(50);

    // Redirect cout to capture output
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    tree.traverse();

    std::cout.rdbuf(old);

    std::string output = buffer.str();
    // Should contain all values
    ASSERT_TRUE(output.find("10") != std::string::npos);
    ASSERT_TRUE(output.find("20") != std::string::npos);
    ASSERT_TRUE(output.find("30") != std::string::npos);
    ASSERT_TRUE(output.find("40") != std::string::npos);
    ASSERT_TRUE(output.find("50") != std::string::npos);
}

// Test: Empty after construction, not empty after insert
TEST(test_empty_state) {
    BTree<int> tree;
    ASSERT_TRUE(tree.empty());
    tree.insert(1);
    ASSERT_FALSE(tree.empty());
}

// Test: Many trees independently
TEST(test_multiple_trees) {
    BTree<int> tree1;
    BTree<int> tree2;

    tree1.insert(10);
    tree2.insert(20);

    ASSERT_TRUE(tree1.search(10));
    ASSERT_FALSE(tree1.search(20));
    ASSERT_TRUE(tree2.search(20));
    ASSERT_FALSE(tree2.search(10));
}

int main() {
    std::cout << "=== BTree Unit Tests ===" << std::endl << std::endl;

    RUN_TEST(test_empty_tree);
    RUN_TEST(test_single_insert);
    RUN_TEST(test_multiple_inserts);
    RUN_TEST(test_sorted_insert);
    RUN_TEST(test_reverse_sorted_insert);
    RUN_TEST(test_many_inserts);
    RUN_TEST(test_random_insert);
    RUN_TEST(test_duplicate_insert);
    RUN_TEST(test_negative_values);
    RUN_TEST(test_string_type);
    RUN_TEST(test_double_type);
    RUN_TEST(test_order_5);
    RUN_TEST(test_order_10);
    RUN_TEST(test_search_empty_tree);
    RUN_TEST(test_boundary_values);
    RUN_TEST(test_stress_shuffled);
    RUN_TEST(test_traverse_order);
    RUN_TEST(test_empty_state);
    RUN_TEST(test_multiple_trees);

    std::cout << std::endl;
    std::cout << "=== Results ===" << std::endl;
    std::cout << "Passed: " << tests_passed << std::endl;
    std::cout << "Failed: " << tests_failed << std::endl;

    return tests_failed == 0 ? 0 : 1;
}
