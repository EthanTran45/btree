#include <iostream>
#include <cassert>
#include <string>
#include <sstream>
#include <vector>
#include <random>
#include <algorithm>
#include <climits>
#include <set>
#include <cstdlib>
#include <new>

// Include the BTree implementation
#include "btree.hpp"

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

// Test: size() method
TEST(test_size) {
    BTree<int> tree;
    ASSERT_EQ(tree.size(), 0u);

    tree.insert(10);
    ASSERT_EQ(tree.size(), 1u);

    tree.insert(20);
    tree.insert(30);
    ASSERT_EQ(tree.size(), 3u);

    for (int i = 0; i < 100; i++) {
        tree.insert(i + 100);
    }
    ASSERT_EQ(tree.size(), 103u);
}

// Test: Basic remove
TEST(test_remove_basic) {
    BTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    ASSERT_TRUE(tree.search(20));
    ASSERT_TRUE(tree.remove(20));
    ASSERT_FALSE(tree.search(20));
    ASSERT_TRUE(tree.search(10));
    ASSERT_TRUE(tree.search(30));
    ASSERT_EQ(tree.size(), 2u);
}

// Test: Remove non-existent key
TEST(test_remove_nonexistent) {
    BTree<int> tree;
    tree.insert(10);

    ASSERT_FALSE(tree.remove(20));
    ASSERT_EQ(tree.size(), 1u);
    ASSERT_TRUE(tree.search(10));
}

// Test: Remove from empty tree
TEST(test_remove_empty) {
    BTree<int> tree;
    ASSERT_FALSE(tree.remove(10));
}

// Test: Remove all elements
TEST(test_remove_all) {
    BTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    ASSERT_TRUE(tree.remove(10));
    ASSERT_TRUE(tree.remove(20));
    ASSERT_TRUE(tree.remove(30));

    ASSERT_TRUE(tree.empty());
    ASSERT_EQ(tree.size(), 0u);
}

// Test: Remove with rebalancing (many elements)
TEST(test_remove_rebalancing) {
    BTree<int, 4> tree;

    // Insert many elements to create a multi-level tree
    for (int i = 0; i < 50; i++) {
        tree.insert(i);
    }

    // Remove elements in various orders
    for (int i = 0; i < 50; i += 2) {
        ASSERT_TRUE(tree.remove(i));
    }

    // Verify remaining elements
    for (int i = 0; i < 50; i++) {
        if (i % 2 == 0) {
            ASSERT_FALSE(tree.search(i));
        } else {
            ASSERT_TRUE(tree.search(i));
        }
    }

    ASSERT_EQ(tree.size(), 25u);
}

// Test: Remove in reverse order
TEST(test_remove_reverse) {
    BTree<int> tree;
    for (int i = 0; i < 20; i++) {
        tree.insert(i);
    }

    for (int i = 19; i >= 0; i--) {
        ASSERT_TRUE(tree.remove(i));
        ASSERT_FALSE(tree.search(i));
    }

    ASSERT_TRUE(tree.empty());
}

// Test: Move constructor
TEST(test_move_constructor) {
    BTree<int> tree1;
    tree1.insert(10);
    tree1.insert(20);
    tree1.insert(30);

    BTree<int> tree2(std::move(tree1));

    ASSERT_TRUE(tree2.search(10));
    ASSERT_TRUE(tree2.search(20));
    ASSERT_TRUE(tree2.search(30));
    ASSERT_EQ(tree2.size(), 3u);

    // Original tree should be empty after move
    ASSERT_TRUE(tree1.empty());
    ASSERT_EQ(tree1.size(), 0u);
}

// Test: Move assignment
TEST(test_move_assignment) {
    BTree<int> tree1;
    tree1.insert(10);
    tree1.insert(20);

    BTree<int> tree2;
    tree2.insert(100);

    tree2 = std::move(tree1);

    ASSERT_TRUE(tree2.search(10));
    ASSERT_TRUE(tree2.search(20));
    ASSERT_FALSE(tree2.search(100));  // Old value should be gone
    ASSERT_EQ(tree2.size(), 2u);

    ASSERT_TRUE(tree1.empty());
}

// Test: Stress test with insert and remove
TEST(test_stress_insert_remove) {
    BTree<int, 5> tree;
    std::vector<int> values;

    for (int i = 0; i < 500; i++) {
        values.push_back(i);
    }

    std::mt19937 rng(123);
    std::shuffle(values.begin(), values.end(), rng);

    // Insert all
    for (int val : values) {
        tree.insert(val);
    }
    ASSERT_EQ(tree.size(), 500u);

    // Shuffle again for removal order
    std::shuffle(values.begin(), values.end(), rng);

    // Remove half
    for (int i = 0; i < 250; i++) {
        ASSERT_TRUE(tree.remove(values[i]));
    }
    ASSERT_EQ(tree.size(), 250u);

    // Verify remaining
    for (int i = 250; i < 500; i++) {
        ASSERT_TRUE(tree.search(values[i]));
    }
}

// === EDGE CASE TESTS ===

// Test: Remove single element tree
TEST(test_remove_single_element) {
    BTree<int> tree;
    tree.insert(42);

    ASSERT_TRUE(tree.remove(42));
    ASSERT_TRUE(tree.empty());
    ASSERT_EQ(tree.size(), 0u);
    ASSERT_FALSE(tree.search(42));
}

// Test: Remove minimum element repeatedly
TEST(test_remove_minimum) {
    BTree<int> tree;
    for (int i = 10; i >= 1; i--) {
        tree.insert(i);
    }

    for (int i = 1; i <= 10; i++) {
        ASSERT_TRUE(tree.remove(i));
        ASSERT_FALSE(tree.search(i));
        // All larger elements should still exist
        for (int j = i + 1; j <= 10; j++) {
            ASSERT_TRUE(tree.search(j));
        }
    }
    ASSERT_TRUE(tree.empty());
}

// Test: Remove maximum element repeatedly
TEST(test_remove_maximum) {
    BTree<int> tree;
    for (int i = 1; i <= 10; i++) {
        tree.insert(i);
    }

    for (int i = 10; i >= 1; i--) {
        ASSERT_TRUE(tree.remove(i));
        ASSERT_FALSE(tree.search(i));
        // All smaller elements should still exist
        for (int j = 1; j < i; j++) {
            ASSERT_TRUE(tree.search(j));
        }
    }
    ASSERT_TRUE(tree.empty());
}

// Test: Re-insert after removal
TEST(test_reinsert_after_remove) {
    BTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    ASSERT_TRUE(tree.remove(20));
    ASSERT_FALSE(tree.search(20));

    tree.insert(20);
    ASSERT_TRUE(tree.search(20));
    ASSERT_EQ(tree.size(), 3u);
}

// Test: Alternating insert and remove
TEST(test_alternating_insert_remove) {
    BTree<int> tree;

    for (int i = 0; i < 100; i++) {
        tree.insert(i);
        tree.insert(i + 100);
        ASSERT_TRUE(tree.remove(i));
    }

    ASSERT_EQ(tree.size(), 100u);

    for (int i = 0; i < 100; i++) {
        ASSERT_FALSE(tree.search(i));
        ASSERT_TRUE(tree.search(i + 100));
    }
}

// Test: Remove middle elements (forces internal node operations)
TEST(test_remove_middle_elements) {
    BTree<int, 4> tree;
    for (int i = 1; i <= 30; i++) {
        tree.insert(i);
    }

    // Remove middle elements
    for (int i = 10; i <= 20; i++) {
        ASSERT_TRUE(tree.remove(i));
    }

    // Verify boundaries still exist
    for (int i = 1; i <= 9; i++) {
        ASSERT_TRUE(tree.search(i));
    }
    for (int i = 21; i <= 30; i++) {
        ASSERT_TRUE(tree.search(i));
    }
}

// Test: Remove same element twice
TEST(test_remove_twice) {
    BTree<int> tree;
    tree.insert(10);

    ASSERT_TRUE(tree.remove(10));
    ASSERT_FALSE(tree.remove(10));  // Second remove should fail
    ASSERT_TRUE(tree.empty());
}

// Test: Minimum order (Order = 3)
TEST(test_order_3_edge_cases) {
    BTree<int, 3> tree;  // max_keys = 2, min_keys = 1

    // Insert enough to create multiple levels
    for (int i = 1; i <= 20; i++) {
        tree.insert(i);
    }

    // Remove in pattern that stresses rebalancing
    ASSERT_TRUE(tree.remove(1));
    ASSERT_TRUE(tree.remove(20));
    ASSERT_TRUE(tree.remove(10));
    ASSERT_TRUE(tree.remove(5));
    ASSERT_TRUE(tree.remove(15));

    ASSERT_EQ(tree.size(), 15u);

    // Verify remaining
    for (int i = 2; i <= 19; i++) {
        if (i != 5 && i != 10 && i != 15) {
            ASSERT_TRUE(tree.search(i));
        }
    }
}

// Test: Large order tree
TEST(test_order_50) {
    BTree<int, 50> tree;

    for (int i = 0; i < 1000; i++) {
        tree.insert(i);
    }

    ASSERT_EQ(tree.size(), 1000u);

    for (int i = 0; i < 500; i++) {
        ASSERT_TRUE(tree.remove(i * 2));  // Remove even numbers
    }

    ASSERT_EQ(tree.size(), 500u);

    for (int i = 0; i < 1000; i++) {
        if (i % 2 == 0) {
            ASSERT_FALSE(tree.search(i));
        } else {
            ASSERT_TRUE(tree.search(i));
        }
    }
}

// Test: Cascade merge scenario
TEST(test_cascade_merge) {
    BTree<int, 3> tree;

    // Build a tree that will require cascade merging
    for (int i = 1; i <= 15; i++) {
        tree.insert(i);
    }

    // Remove elements that will trigger merges
    for (int i = 1; i <= 15; i++) {
        ASSERT_TRUE(tree.remove(i));
    }

    ASSERT_TRUE(tree.empty());
}

// Test: Insert after complete removal
TEST(test_insert_after_empty) {
    BTree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);

    tree.remove(1);
    tree.remove(2);
    tree.remove(3);

    ASSERT_TRUE(tree.empty());

    // Tree should work normally after being emptied
    tree.insert(100);
    tree.insert(200);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(tree.size(), 2u);
    ASSERT_TRUE(tree.search(100));
    ASSERT_TRUE(tree.search(200));
}

// Test: String edge cases
TEST(test_string_edge_cases) {
    BTree<std::string> tree;

    tree.insert("");  // Empty string
    tree.insert("a");
    tree.insert("aa");
    tree.insert("aaa");
    tree.insert("b");

    ASSERT_TRUE(tree.search(""));
    ASSERT_TRUE(tree.remove(""));
    ASSERT_FALSE(tree.search(""));

    ASSERT_TRUE(tree.remove("aa"));
    ASSERT_TRUE(tree.search("a"));
    ASSERT_TRUE(tree.search("aaa"));
}

// Test: Size consistency after failed removes
TEST(test_size_consistency) {
    BTree<int> tree;
    for (int i = 0; i < 10; i++) {
        tree.insert(i);
    }

    size_t expected_size = 10;
    ASSERT_EQ(tree.size(), expected_size);

    // Failed removes shouldn't change size
    ASSERT_FALSE(tree.remove(100));
    ASSERT_FALSE(tree.remove(-1));
    ASSERT_FALSE(tree.remove(50));
    ASSERT_EQ(tree.size(), expected_size);

    // Successful remove should decrement
    ASSERT_TRUE(tree.remove(5));
    ASSERT_EQ(tree.size(), expected_size - 1);
}

// Test: Interleaved operations stress test
TEST(test_interleaved_stress) {
    BTree<int, 4> tree;
    std::mt19937 rng(999);

    std::vector<int> present;

    for (int round = 0; round < 10; round++) {
        // Insert 50 elements
        for (int i = 0; i < 50; i++) {
            int val = round * 100 + i;
            tree.insert(val);
            present.push_back(val);
        }

        // Remove 25 random elements
        std::shuffle(present.begin(), present.end(), rng);
        for (int i = 0; i < 25 && !present.empty(); i++) {
            int val = present.back();
            present.pop_back();
            ASSERT_TRUE(tree.remove(val));
        }
    }

    // Verify all remaining elements
    ASSERT_EQ(tree.size(), present.size());
    for (int val : present) {
        ASSERT_TRUE(tree.search(val));
    }
}

// Test: Verify tree integrity after random operations
TEST(test_random_operations_integrity) {
    BTree<int, 5> tree;
    std::set<int> present;  // Track unique values in tree
    std::mt19937 rng(42);

    // Only insert unique values to avoid duplicate complexity
    for (int i = 0; i < 500; i++) {
        int val = rng() % 300;  // Larger range to reduce collisions
        if (rng() % 2 == 0) {
            // Only insert if not present (to match set behavior)
            if (present.find(val) == present.end()) {
                tree.insert(val);
                present.insert(val);
            }
        } else {
            bool tree_result = tree.remove(val);
            bool ref_result = present.erase(val) > 0;
            ASSERT_EQ(tree_result, ref_result);
        }
    }

    // Verify final state matches reference
    ASSERT_EQ(tree.size(), present.size());
    for (int val : present) {
        ASSERT_TRUE(tree.search(val));
    }
}

// === NEW API TESTS ===

// Test: contains() method (alias for search)
TEST(test_contains) {
    BTree<int> tree;
    tree.insert(10);
    tree.insert(20);

    ASSERT_TRUE(tree.contains(10));
    ASSERT_TRUE(tree.contains(20));
    ASSERT_FALSE(tree.contains(30));
}

// Test: clear() method
TEST(test_clear) {
    BTree<int> tree;
    for (int i = 0; i < 50; i++) {
        tree.insert(i);
    }

    ASSERT_EQ(tree.size(), 50u);
    ASSERT_FALSE(tree.empty());

    tree.clear();

    ASSERT_EQ(tree.size(), 0u);
    ASSERT_TRUE(tree.empty());
    ASSERT_FALSE(tree.contains(25));

    // Should work normally after clear
    tree.insert(100);
    ASSERT_EQ(tree.size(), 1u);
    ASSERT_TRUE(tree.contains(100));
}

// Test: height() method
TEST(test_height) {
    BTree<int, 3> tree;  // Order 3: max 2 keys per node

    ASSERT_EQ(tree.height(), 0u);  // Empty tree

    tree.insert(10);
    ASSERT_EQ(tree.height(), 1u);  // Single node

    // Insert more to force splits and increase height
    for (int i = 0; i < 20; i++) {
        tree.insert(i);
    }
    ASSERT_TRUE(tree.height() >= 2u);  // Should have multiple levels
}

// Test: min() method
TEST(test_min) {
    BTree<int> tree;

    // Test exception on empty tree
    bool threw = false;
    try {
        (void)tree.min();
    } catch (const std::runtime_error&) {
        threw = true;
    }
    ASSERT_TRUE(threw);

    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.insert(10);
    tree.insert(90);

    ASSERT_EQ(tree.min(), 10);

    tree.remove(10);
    ASSERT_EQ(tree.min(), 30);
}

// Test: max() method
TEST(test_max) {
    BTree<int> tree;

    // Test exception on empty tree
    bool threw = false;
    try {
        (void)tree.max();
    } catch (const std::runtime_error&) {
        threw = true;
    }
    ASSERT_TRUE(threw);

    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.insert(10);
    tree.insert(90);

    ASSERT_EQ(tree.max(), 90);

    tree.remove(90);
    ASSERT_EQ(tree.max(), 70);
}

// Test: for_each() method
TEST(test_for_each) {
    BTree<int> tree;
    tree.insert(30);
    tree.insert(10);
    tree.insert(20);
    tree.insert(40);

    std::vector<int> collected;
    tree.for_each([&collected](int val) {
        collected.push_back(val);
    });

    ASSERT_EQ(collected.size(), 4u);
    // Should be in sorted order
    ASSERT_EQ(collected[0], 10);
    ASSERT_EQ(collected[1], 20);
    ASSERT_EQ(collected[2], 30);
    ASSERT_EQ(collected[3], 40);
}

// Test: to_vector() method
TEST(test_to_vector) {
    BTree<int> tree;
    tree.insert(50);
    tree.insert(25);
    tree.insert(75);
    tree.insert(10);
    tree.insert(30);

    std::vector<int> vec = tree.to_vector();

    ASSERT_EQ(vec.size(), 5u);
    // Should be sorted
    ASSERT_EQ(vec[0], 10);
    ASSERT_EQ(vec[1], 25);
    ASSERT_EQ(vec[2], 30);
    ASSERT_EQ(vec[3], 50);
    ASSERT_EQ(vec[4], 75);
}

// Test: Iterator basic usage
TEST(test_iterator_basic) {
    BTree<int> tree;
    tree.insert(30);
    tree.insert(10);
    tree.insert(20);

    std::vector<int> collected;
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        collected.push_back(*it);
    }

    ASSERT_EQ(collected.size(), 3u);
    ASSERT_EQ(collected[0], 10);
    ASSERT_EQ(collected[1], 20);
    ASSERT_EQ(collected[2], 30);
}

// Test: Range-based for loop
TEST(test_range_based_for) {
    BTree<int> tree;
    tree.insert(5);
    tree.insert(3);
    tree.insert(7);
    tree.insert(1);
    tree.insert(9);

    std::vector<int> collected;
    for (const auto& val : tree) {
        collected.push_back(val);
    }

    ASSERT_EQ(collected.size(), 5u);
    ASSERT_EQ(collected[0], 1);
    ASSERT_EQ(collected[1], 3);
    ASSERT_EQ(collected[2], 5);
    ASSERT_EQ(collected[3], 7);
    ASSERT_EQ(collected[4], 9);
}

// Test: Iterator with large tree
TEST(test_iterator_large) {
    BTree<int, 4> tree;

    for (int i = 99; i >= 0; i--) {
        tree.insert(i);
    }

    int expected = 0;
    for (const auto& val : tree) {
        ASSERT_EQ(val, expected);
        expected++;
    }
    ASSERT_EQ(expected, 100);
}

// Test: Iterator on empty tree
TEST(test_iterator_empty) {
    BTree<int> tree;

    int count = 0;
    for (const auto& val : tree) {
        (void)val;
        count++;
    }
    ASSERT_EQ(count, 0);

    ASSERT_TRUE(tree.begin() == tree.end());
}

// Test: cbegin/cend
TEST(test_const_iterators) {
    BTree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);

    std::vector<int> collected;
    for (auto it = tree.cbegin(); it != tree.cend(); ++it) {
        collected.push_back(*it);
    }

    ASSERT_EQ(collected.size(), 3u);
}

// Test: traverse with custom ostream
TEST(test_traverse_ostream) {
    BTree<int> tree;
    tree.insert(30);
    tree.insert(10);
    tree.insert(20);

    std::stringstream ss;
    tree.traverse(ss);

    std::string output = ss.str();
    ASSERT_TRUE(output.find("10") != std::string::npos);
    ASSERT_TRUE(output.find("20") != std::string::npos);
    ASSERT_TRUE(output.find("30") != std::string::npos);
}

// Test: Iterator post-increment
TEST(test_iterator_post_increment) {
    BTree<int> tree;
    tree.insert(10);
    tree.insert(20);

    auto it = tree.begin();
    auto old = it++;

    ASSERT_EQ(*old, 10);
    ASSERT_EQ(*it, 20);
}

// Test: STL algorithm compatibility
TEST(test_stl_algorithms) {
    BTree<int> tree;
    for (int i = 1; i <= 10; i++) {
        tree.insert(i);
    }

    // std::find
    auto it = std::find(tree.begin(), tree.end(), 5);
    ASSERT_TRUE(it != tree.end());
    ASSERT_EQ(*it, 5);

    // std::count
    int count = std::count(tree.begin(), tree.end(), 7);
    ASSERT_EQ(count, 1);

    // std::accumulate (requires numeric header but we can do manual sum)
    int sum = 0;
    for (const auto& val : tree) {
        sum += val;
    }
    ASSERT_EQ(sum, 55);  // 1+2+...+10
}

// Test: find() method returning iterator
TEST(test_find_method) {
    BTree<int> tree;
    for (int i = 1; i <= 10; i++) {
        tree.insert(i * 10);
    }

    // Find existing element
    auto it = tree.find(50);
    ASSERT_TRUE(it != tree.end());
    ASSERT_EQ(*it, 50);

    // Find non-existing element
    auto it2 = tree.find(55);
    ASSERT_TRUE(it2 == tree.end());

    // Find first element
    auto it3 = tree.find(10);
    ASSERT_TRUE(it3 != tree.end());
    ASSERT_EQ(*it3, 10);

    // Find last element
    auto it4 = tree.find(100);
    ASSERT_TRUE(it4 != tree.end());
    ASSERT_EQ(*it4, 100);

    // Find on empty tree
    BTree<int> empty_tree;
    auto it5 = empty_tree.find(42);
    ASSERT_TRUE(it5 == empty_tree.end());
}

// === ADDITIONAL TESTS ===

// === Empty Tree Edge Cases ===

// Test: to_vector on empty tree
TEST(test_to_vector_empty) {
    BTree<int> tree;
    std::vector<int> vec = tree.to_vector();
    ASSERT_TRUE(vec.empty());
    ASSERT_EQ(vec.size(), 0u);
}

// Test: for_each on empty tree
TEST(test_for_each_empty) {
    BTree<int> tree;
    int count = 0;
    tree.for_each([&count](int) {
        count++;
    });
    ASSERT_EQ(count, 0);
}

// Test: traverse on empty tree (should not crash)
TEST(test_traverse_empty) {
    BTree<int> tree;
    std::stringstream ss;
    tree.traverse(ss);
    // Should produce empty or just newline
    ASSERT_TRUE(ss.str().empty() || ss.str() == "\n" || ss.str().find_first_not_of(" \n\t") == std::string::npos);
}

// === Iterator Edge Cases ===

// Test: iterator arrow operator
TEST(test_iterator_arrow_operator) {
    BTree<std::string> tree;
    tree.insert("hello");
    tree.insert("world");

    auto it = tree.begin();
    // Use arrow operator to access string methods
    ASSERT_EQ(it->length(), 5u);  // "hello" has length 5
    ASSERT_EQ(it->at(0), 'h');
}

// Test: iterator on single element tree
TEST(test_iterator_single_element) {
    BTree<int> tree;
    tree.insert(42);

    auto it = tree.begin();
    ASSERT_TRUE(it != tree.end());
    ASSERT_EQ(*it, 42);

    ++it;
    ASSERT_TRUE(it == tree.end());
}

// Test: iterator equality and inequality
TEST(test_iterator_equality) {
    BTree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);

    auto it1 = tree.begin();
    auto it2 = tree.begin();

    // Same position should be equal
    ASSERT_TRUE(it1 == it2);
    ASSERT_FALSE(it1 != it2);

    ++it1;
    // Different positions should not be equal
    ASSERT_FALSE(it1 == it2);
    ASSERT_TRUE(it1 != it2);

    ++it2;
    // Same position again
    ASSERT_TRUE(it1 == it2);

    // End iterators should be equal
    auto end1 = tree.end();
    auto end2 = tree.end();
    ASSERT_TRUE(end1 == end2);
}

// Test: iterator with string type using arrow operator
TEST(test_iterator_string_arrow) {
    BTree<std::string> tree;
    tree.insert("apple");
    tree.insert("banana");
    tree.insert("cherry");

    std::vector<size_t> lengths;
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        lengths.push_back(it->size());
    }

    ASSERT_EQ(lengths.size(), 3u);
    ASSERT_EQ(lengths[0], 5u);  // apple
    ASSERT_EQ(lengths[1], 6u);  // banana
    ASSERT_EQ(lengths[2], 6u);  // cherry
}

// === Duplicate Handling Tests ===

// Test: duplicate behavior - verify size increases (multi-set semantics)
TEST(test_duplicate_size_behavior) {
    BTree<int> tree;
    tree.insert(10);
    ASSERT_EQ(tree.size(), 1u);

    tree.insert(10);  // Duplicate
    ASSERT_EQ(tree.size(), 2u);

    tree.insert(10);  // Another duplicate
    ASSERT_EQ(tree.size(), 3u);

    ASSERT_TRUE(tree.search(10));
}

// Test: multiple duplicates of same key
TEST(test_multiple_duplicates) {
    BTree<int> tree;

    // Insert 100 copies of the same key
    for (int i = 0; i < 100; i++) {
        tree.insert(42);
    }

    ASSERT_EQ(tree.size(), 100u);
    ASSERT_TRUE(tree.search(42));

    // Verify iteration counts all duplicates
    int count = 0;
    for (const auto& val : tree) {
        ASSERT_EQ(val, 42);
        count++;
    }
    ASSERT_EQ(count, 100);
}

// Test: remove only removes one duplicate
TEST(test_remove_one_duplicate) {
    BTree<int> tree;
    tree.insert(10);
    tree.insert(10);
    tree.insert(10);

    ASSERT_EQ(tree.size(), 3u);

    ASSERT_TRUE(tree.remove(10));
    ASSERT_EQ(tree.size(), 2u);
    ASSERT_TRUE(tree.search(10));  // Should still find remaining duplicates

    ASSERT_TRUE(tree.remove(10));
    ASSERT_EQ(tree.size(), 1u);
    ASSERT_TRUE(tree.search(10));

    ASSERT_TRUE(tree.remove(10));
    ASSERT_EQ(tree.size(), 0u);
    ASSERT_FALSE(tree.search(10));
}

// === Move Semantics Edge Cases ===

// Test: self-move assignment (should be safe)
TEST(test_self_move_assignment) {
    BTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    // Self-move assignment - implementation checks `this != &other` so tree is preserved.
    // The self-move is deliberate here; silence the (correct) -Wself-move warning.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
#endif
    tree = std::move(tree);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

    // Tree should be preserved (self-assignment is a no-op)
    ASSERT_EQ(tree.size(), 3u);
    ASSERT_TRUE(tree.search(10));
    ASSERT_TRUE(tree.search(20));
    ASSERT_TRUE(tree.search(30));

    // Verify tree is still functional
    tree.insert(40);
    ASSERT_EQ(tree.size(), 4u);
    ASSERT_TRUE(tree.search(40));
}

// Test: move from empty tree
TEST(test_move_empty_tree) {
    BTree<int> empty_tree;
    BTree<int> tree2(std::move(empty_tree));

    ASSERT_TRUE(tree2.empty());
    ASSERT_EQ(tree2.size(), 0u);
    ASSERT_TRUE(empty_tree.empty());

    // Should work normally after move
    tree2.insert(100);
    ASSERT_EQ(tree2.size(), 1u);
    ASSERT_TRUE(tree2.search(100));
}

// === Height Verification Tests ===

// Test: verify height grows with insertions
TEST(test_height_growth) {
    BTree<int, 3> tree;  // Order 3: max 2 keys per node

    ASSERT_EQ(tree.height(), 0u);

    tree.insert(1);
    ASSERT_EQ(tree.height(), 1u);

    tree.insert(2);
    ASSERT_EQ(tree.height(), 1u);  // Still fits in root

    // After 3rd insert, should split
    tree.insert(3);
    size_t h1 = tree.height();

    // Insert many more elements
    for (int i = 4; i <= 20; i++) {
        tree.insert(i);
    }
    size_t h2 = tree.height();

    // Height should have increased
    ASSERT_TRUE(h2 >= h1);
    ASSERT_TRUE(h2 >= 2u);
}

// Test: height after removals
TEST(test_height_after_removals) {
    BTree<int, 4> tree;

    // Build up a tree
    for (int i = 0; i < 100; i++) {
        tree.insert(i);
    }
    size_t initial_height = tree.height();
    ASSERT_TRUE(initial_height >= 2u);

    // Remove most elements
    for (int i = 0; i < 90; i++) {
        tree.remove(i);
    }

    // Height should have decreased or stayed same
    size_t final_height = tree.height();
    ASSERT_TRUE(final_height <= initial_height);
    ASSERT_TRUE(final_height >= 1u);
}

// === Min/Max Edge Cases ===

// Test: min/max through many modifications
TEST(test_min_max_through_modifications) {
    BTree<int> tree;

    tree.insert(50);
    ASSERT_EQ(tree.min(), 50);
    ASSERT_EQ(tree.max(), 50);

    tree.insert(25);
    ASSERT_EQ(tree.min(), 25);
    ASSERT_EQ(tree.max(), 50);

    tree.insert(75);
    ASSERT_EQ(tree.min(), 25);
    ASSERT_EQ(tree.max(), 75);

    tree.insert(10);
    tree.insert(90);
    ASSERT_EQ(tree.min(), 10);
    ASSERT_EQ(tree.max(), 90);

    // Remove min
    tree.remove(10);
    ASSERT_EQ(tree.min(), 25);

    // Remove max
    tree.remove(90);
    ASSERT_EQ(tree.max(), 75);

    // Remove middle elements - min/max should be unchanged
    tree.remove(50);
    ASSERT_EQ(tree.min(), 25);
    ASSERT_EQ(tree.max(), 75);
}

// Test: min/max with negative and positive values
TEST(test_min_max_negative_positive) {
    BTree<int> tree;

    tree.insert(0);
    tree.insert(-100);
    tree.insert(100);
    tree.insert(-50);
    tree.insert(50);

    ASSERT_EQ(tree.min(), -100);
    ASSERT_EQ(tree.max(), 100);

    tree.remove(-100);
    ASSERT_EQ(tree.min(), -50);

    tree.remove(100);
    ASSERT_EQ(tree.max(), 50);
}

// === Clear Edge Cases ===

// Test: clear on already empty tree
TEST(test_clear_empty_tree) {
    BTree<int> tree;
    ASSERT_TRUE(tree.empty());

    tree.clear();  // Should not crash
    ASSERT_TRUE(tree.empty());
    ASSERT_EQ(tree.size(), 0u);

    // Should still work after clear
    tree.insert(100);
    ASSERT_EQ(tree.size(), 1u);
}

// Test: multiple clear calls
TEST(test_multiple_clears) {
    BTree<int> tree;

    for (int i = 0; i < 50; i++) {
        tree.insert(i);
    }
    ASSERT_EQ(tree.size(), 50u);

    tree.clear();
    ASSERT_TRUE(tree.empty());

    tree.clear();  // Second clear on empty
    ASSERT_TRUE(tree.empty());

    // Insert again
    for (int i = 0; i < 25; i++) {
        tree.insert(i);
    }
    ASSERT_EQ(tree.size(), 25u);

    tree.clear();
    tree.clear();
    tree.clear();  // Multiple clears

    ASSERT_TRUE(tree.empty());
}

// === Find Edge Cases ===

// Test: find with duplicates
TEST(test_find_with_duplicates) {
    BTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(10);  // Duplicate
    tree.insert(30);
    tree.insert(10);  // Another duplicate

    auto it = tree.find(10);
    ASSERT_TRUE(it != tree.end());
    ASSERT_EQ(*it, 10);

    // Find should return an iterator to one of the 10s
    int count = 0;
    for (auto i = tree.begin(); i != tree.end(); ++i) {
        if (*i == 10) count++;
    }
    ASSERT_EQ(count, 3);  // All duplicates should be in tree
}

// Test: find() returns the FIRST equal element, so iterating [find(k), end())
// visits every duplicate of k and agrees with std::find over the same range.
// Regression for a bug where find() could land on a middle duplicate (one
// promoted to an internal separator), silently skipping earlier equal copies.
// Uses a small order and many duplicates to force keys into internal nodes.
TEST(test_find_returns_first_duplicate) {
    for (int trial = 0; trial < 2; ++trial) {
        BTree<int, 3> tree;               // small order => deep tree, internal dups
        std::multiset<int> ref;
        // Interleave copies so duplicates spread across nodes/levels.
        for (int rep = 0; rep < 6; ++rep) {
            for (int k = 0; k < 10; ++k) {
                if (trial == 1 && (rep + k) % 2) continue;  // vary the shape
                tree.insert(k);
                ref.insert(k);
            }
        }
        std::vector<int> sorted(ref.begin(), ref.end());

        for (int k = 0; k < 10; ++k) {
            long total = std::count(sorted.begin(), sorted.end(), k);
            auto it = tree.find(k);
            if (total == 0) { ASSERT_TRUE(it == tree.end()); continue; }

            ASSERT_TRUE(it != tree.end());
            ASSERT_EQ(*it, k);

            // Walking forward from find(k) must reach every copy of k...
            long reachable = 0;
            for (auto j = it; j != tree.end() && *j == k; ++j) ++reachable;
            ASSERT_EQ(reachable, total);

            // ...and must be positioned exactly where std::find lands (first
            // equal element), i.e. the remaining sequences are identical.
            std::vector<int> from_find(it, tree.end());
            std::vector<int> from_stdfind(std::find(tree.begin(), tree.end(), k),
                                          tree.end());
            ASSERT_TRUE(from_find == from_stdfind);
            // The suffix from the first occurrence is exactly sorted[first..].
            auto first = std::lower_bound(sorted.begin(), sorted.end(), k);
            ASSERT_TRUE(from_find == std::vector<int>(first, sorted.end()));
        }
    }
}

// Test: find after modifications
TEST(test_find_after_modifications) {
    BTree<int> tree;
    for (int i = 1; i <= 20; i++) {
        tree.insert(i * 5);
    }

    // Find before modification
    auto it1 = tree.find(50);
    ASSERT_TRUE(it1 != tree.end());

    // Remove element
    tree.remove(50);
    auto it2 = tree.find(50);
    ASSERT_TRUE(it2 == tree.end());

    // Re-insert
    tree.insert(50);
    auto it3 = tree.find(50);
    ASSERT_TRUE(it3 != tree.end());
    ASSERT_EQ(*it3, 50);
}

// === Higher-Order Remove Stress Tests ===

// Test: Order 4 random remove stress
TEST(test_order_4_random_remove_stress) {
    BTree<int, 4> tree;
    std::vector<int> values;

    for (int i = 0; i < 1000; i++) {
        values.push_back(i);
    }

    std::mt19937 rng(12345);
    std::shuffle(values.begin(), values.end(), rng);

    // Insert all
    for (int val : values) {
        tree.insert(val);
    }
    ASSERT_EQ(tree.size(), 1000u);

    // Shuffle for removal
    std::shuffle(values.begin(), values.end(), rng);

    // Remove all in random order
    for (int val : values) {
        ASSERT_TRUE(tree.remove(val));
    }

    ASSERT_TRUE(tree.empty());
}

// Test: Order 6 sequential remove
TEST(test_order_6_remove_sequential) {
    BTree<int, 6> tree;

    for (int i = 0; i < 500; i++) {
        tree.insert(i);
    }
    ASSERT_EQ(tree.size(), 500u);

    // Remove in forward order
    for (int i = 0; i < 500; i++) {
        ASSERT_TRUE(tree.remove(i));
        ASSERT_FALSE(tree.search(i));
    }

    ASSERT_TRUE(tree.empty());
}

// Test: Order 7 interleaved stress
TEST(test_order_7_interleaved_stress) {
    BTree<int, 7> tree;
    std::set<int> reference;
    std::mt19937 rng(54321);

    for (int i = 0; i < 2000; i++) {
        int val = rng() % 500;
        if (rng() % 3 != 0) {
            // Insert (2/3 of operations)
            if (reference.find(val) == reference.end()) {
                tree.insert(val);
                reference.insert(val);
            }
        } else {
            // Remove (1/3 of operations)
            bool tree_result = tree.remove(val);
            bool ref_result = reference.erase(val) > 0;
            ASSERT_EQ(tree_result, ref_result);
        }
    }

    // Verify final state
    ASSERT_EQ(tree.size(), reference.size());
    for (int val : reference) {
        ASSERT_TRUE(tree.search(val));
    }
}

// === Large Scale Stress Tests ===

// Test: 10,000 elements
TEST(test_large_scale_10k) {
    BTree<int, 5> tree;

    for (int i = 0; i < 10000; i++) {
        tree.insert(i);
    }
    ASSERT_EQ(tree.size(), 10000u);

    // Verify all present
    for (int i = 0; i < 10000; i++) {
        ASSERT_TRUE(tree.search(i));
    }

    // Remove half
    for (int i = 0; i < 5000; i++) {
        ASSERT_TRUE(tree.remove(i * 2));  // Remove evens
    }
    ASSERT_EQ(tree.size(), 5000u);

    // Verify state
    for (int i = 0; i < 10000; i++) {
        if (i % 2 == 0) {
            ASSERT_FALSE(tree.search(i));
        } else {
            ASSERT_TRUE(tree.search(i));
        }
    }
}

// Test: 50,000 elements with Order 10
TEST(test_large_scale_50k_order_10) {
    BTree<int, 10> tree;
    std::vector<int> values;

    for (int i = 0; i < 50000; i++) {
        values.push_back(i);
    }

    std::mt19937 rng(99999);
    std::shuffle(values.begin(), values.end(), rng);

    for (int val : values) {
        tree.insert(val);
    }
    ASSERT_EQ(tree.size(), 50000u);

    // Verify via iteration
    int count = 0;
    int prev = -1;
    for (const auto& val : tree) {
        ASSERT_TRUE(val > prev);  // Should be sorted
        prev = val;
        count++;
    }
    ASSERT_EQ(count, 50000);
}

// === String Stress Tests ===

// Test: very long strings
TEST(test_long_strings) {
    BTree<std::string> tree;

    std::string long1(1000, 'a');
    std::string long2(1000, 'b');
    std::string long3(1000, 'c');
    std::string medium(500, 'x');

    tree.insert(long2);
    tree.insert(long1);
    tree.insert(long3);
    tree.insert(medium);

    ASSERT_EQ(tree.size(), 4u);
    ASSERT_TRUE(tree.search(long1));
    ASSERT_TRUE(tree.search(long2));
    ASSERT_TRUE(tree.search(long3));
    ASSERT_TRUE(tree.search(medium));

    ASSERT_EQ(tree.min(), long1);  // 'aaa...' < 'bbb...'
    ASSERT_EQ(tree.max(), medium); // 'xxx...' > 'ccc...'

    ASSERT_TRUE(tree.remove(long2));
    ASSERT_FALSE(tree.search(long2));
}

// Test: special string patterns
TEST(test_special_string_patterns) {
    BTree<std::string> tree;

    // Various edge case strings
    tree.insert("");           // Empty string
    tree.insert(" ");          // Single space
    tree.insert("  ");         // Two spaces
    tree.insert("\t");         // Tab
    tree.insert("\n");         // Newline
    tree.insert("0");
    tree.insert("00");
    tree.insert("000");
    tree.insert("a");
    tree.insert("A");          // Case sensitivity
    tree.insert("Z");
    tree.insert("z");

    ASSERT_EQ(tree.size(), 12u);

    // Verify all present
    ASSERT_TRUE(tree.search(""));
    ASSERT_TRUE(tree.search(" "));
    ASSERT_TRUE(tree.search("\t"));
    ASSERT_TRUE(tree.search("A"));
    ASSERT_TRUE(tree.search("a"));

    // Empty string should be minimum
    ASSERT_EQ(tree.min(), "");
}

// === Custom Type Test ===

// Custom struct with only operator<
struct Point {
    int x, y;

    bool operator<(const Point& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    bool operator>(const Point& other) const {
        return other < *this;
    }
};

// Test: custom comparable type
TEST(test_custom_comparable_type) {
    BTree<Point> tree;

    tree.insert({0, 0});
    tree.insert({1, 0});
    tree.insert({0, 1});
    tree.insert({1, 1});
    tree.insert({-1, -1});

    ASSERT_EQ(tree.size(), 5u);

    ASSERT_TRUE(tree.search({0, 0}));
    ASSERT_TRUE(tree.search({1, 1}));
    ASSERT_TRUE(tree.search({-1, -1}));
    ASSERT_FALSE(tree.search({2, 2}));

    Point min_point = tree.min();
    ASSERT_EQ(min_point.x, -1);
    ASSERT_EQ(min_point.y, -1);

    Point max_point = tree.max();
    ASSERT_EQ(max_point.x, 1);
    ASSERT_EQ(max_point.y, 1);

    ASSERT_TRUE(tree.remove({0, 0}));
    ASSERT_FALSE(tree.search({0, 0}));
    ASSERT_EQ(tree.size(), 4u);
}

// === Traverse Order Verification ===

// Test: verify traverse outputs in strictly sorted order
TEST(test_traverse_strict_order) {
    BTree<int> tree;
    std::vector<int> input = {50, 25, 75, 10, 30, 60, 90, 5, 15, 27, 35};

    for (int val : input) {
        tree.insert(val);
    }

    std::stringstream ss;
    tree.traverse(ss);

    // Parse output and verify sorted
    std::vector<int> output;
    int val;
    while (ss >> val) {
        output.push_back(val);
    }

    ASSERT_EQ(output.size(), input.size());

    // Check strictly sorted
    for (size_t i = 1; i < output.size(); i++) {
        ASSERT_TRUE(output[i] > output[i-1]);
    }

    // Check contains same elements
    std::sort(input.begin(), input.end());
    ASSERT_TRUE(output == input);
}

// === EDGE CONDITION TESTS ===

// === Tier 1: Critical Edge Cases ===

// Test: Root collapse when internal root has single child after merge
TEST(test_root_collapse_internal) {
    BTree<int, 4> tree;  // Order 4: min_keys=1, max_keys=3

    // Insert enough to create multi-level tree
    for (int i = 1; i <= 10; i++) {
        tree.insert(i);
    }

    size_t h_before = tree.height();
    ASSERT_TRUE(h_before >= 2u);

    // Remove elements strategically to force merges and root collapse
    for (int i = 1; i <= 8; i++) {
        ASSERT_TRUE(tree.remove(i));
    }

    // Height should have reduced
    ASSERT_TRUE(tree.height() <= h_before);

    // Remaining elements should still be findable
    ASSERT_TRUE(tree.search(9));
    ASSERT_TRUE(tree.search(10));
    ASSERT_EQ(tree.size(), 2u);
}

// Test: Case 2c remove - key pushed back up then removed recursively
TEST(test_remove_case_2c_recursive) {
    BTree<int, 5> tree;  // Order 5 for more reliability

    // Build tree with specific structure
    for (int i = 1; i <= 30; i++) {
        tree.insert(i);
    }

    std::set<int> present;
    for (int i = 1; i <= 30; i++) {
        present.insert(i);
    }

    // Remove keys from middle that are likely internal node keys
    std::vector<int> remove_order = {15, 10, 20, 5, 25, 8, 12, 18, 22};
    for (int key : remove_order) {
        if (present.count(key)) {
            bool removed = tree.remove(key);
            ASSERT_TRUE(removed);
            present.erase(key);
            ASSERT_EQ(tree.size(), present.size());
        }
    }

    // Verify all remaining keys still searchable
    for (int val : present) {
        ASSERT_TRUE(tree.search(val));
    }

    // Verify iteration works
    int count = 0;
    for (const auto& v : tree) {
        (void)v;
        count++;
    }
    ASSERT_EQ(static_cast<size_t>(count), present.size());
}

// Test: Order 4 sustained merge-split cycles
TEST(test_order_4_merge_split_cycle) {
    BTree<int, 4> tree;  // Order 4: min_keys=1, max_keys=3

    // Insert elements
    for (int i = 1; i <= 30; i++) {
        tree.insert(i);
    }

    // Repeatedly insert and remove to trigger merge-split cycles
    std::set<int> present;
    for (int i = 1; i <= 30; i++) {
        present.insert(i);
    }

    for (int round = 0; round < 5; round++) {
        // Remove some elements
        for (int i = 1; i <= 5; i++) {
            int to_remove = round * 5 + i;
            if (present.count(to_remove)) {
                tree.remove(to_remove);
                present.erase(to_remove);
            }
        }

        // Insert new elements
        for (int i = 1; i <= 5; i++) {
            int to_insert = 100 + round * 5 + i;
            tree.insert(to_insert);
            present.insert(to_insert);
        }
    }

    // Tree should still be valid
    ASSERT_EQ(tree.size(), present.size());

    // Verify iteration works (tests tree integrity)
    int count = 0;
    for (const auto& val : tree) {
        ASSERT_TRUE(present.count(val) > 0);
        count++;
    }
    ASSERT_EQ(static_cast<size_t>(count), present.size());
}

// Test: Borrow from right sibling specifically
TEST(test_borrow_from_right_sibling) {
    BTree<int, 4> tree;  // Order 4

    // Insert elements to create predictable structure
    for (int i = 1; i <= 20; i++) {
        tree.insert(i);
    }

    // Remove from left side to deplete left children
    // This should force borrow from right sibling
    ASSERT_TRUE(tree.remove(1));
    ASSERT_TRUE(tree.remove(2));
    ASSERT_TRUE(tree.remove(3));

    // Verify remaining elements
    for (int i = 4; i <= 20; i++) {
        ASSERT_TRUE(tree.search(i));
    }

    ASSERT_EQ(tree.size(), 17u);
}

// Test: Rightmost child rebalancing via merge with left
TEST(test_rightmost_child_merge_left) {
    BTree<int, 4> tree;  // Order 4

    // Insert elements
    for (int i = 1; i <= 15; i++) {
        tree.insert(i);
    }

    // Remove from the right side to trigger rightmost child rebalancing
    ASSERT_TRUE(tree.remove(15));
    ASSERT_TRUE(tree.remove(14));
    ASSERT_TRUE(tree.remove(13));

    // Verify remaining elements
    for (int i = 1; i <= 12; i++) {
        ASSERT_TRUE(tree.search(i));
    }

    ASSERT_EQ(tree.size(), 12u);
}

// Test: Iterator behavior after remove operations
TEST(test_iterator_after_remove) {
    BTree<int> tree;
    for (int i = 1; i <= 10; i++) {
        tree.insert(i * 10);
    }

    // Get values via iteration BEFORE any modification
    std::vector<int> before;
    for (const auto& v : tree) {
        before.push_back(v);
    }
    ASSERT_EQ(before.size(), 10u);

    // Remove middle element
    tree.remove(50);

    // Create new iterator and verify it works correctly
    std::vector<int> after;
    for (const auto& v : tree) {
        after.push_back(v);
    }
    ASSERT_EQ(after.size(), 9u);

    // Verify 50 is not in result
    ASSERT_TRUE(std::find(after.begin(), after.end(), 50) == after.end());

    // Verify sorted order maintained
    for (size_t i = 1; i < after.size(); i++) {
        ASSERT_TRUE(after[i] > after[i-1]);
    }
}

// Test: Order 4 comprehensive (no Order 4 tests existed)
TEST(test_order_4_comprehensive) {
    BTree<int, 4> tree;  // Order 4: min_keys=1, max_keys=3

    std::vector<int> values;
    for (int i = 0; i < 200; i++) {
        values.push_back(i);
    }

    std::mt19937 rng(777);
    std::shuffle(values.begin(), values.end(), rng);

    // Insert all in random order
    for (int val : values) {
        tree.insert(val);
    }
    ASSERT_EQ(tree.size(), 200u);

    // Verify all present
    for (int i = 0; i < 200; i++) {
        ASSERT_TRUE(tree.search(i));
    }

    // Remove all in random order
    std::shuffle(values.begin(), values.end(), rng);
    for (int val : values) {
        ASSERT_TRUE(tree.remove(val));
    }
    ASSERT_TRUE(tree.empty());
}

// Test: Root split when root is full leaf
TEST(test_root_split_full_leaf) {
    BTree<int, 4> tree;  // Order 4: max_keys=3

    // Insert exactly max_keys elements
    tree.insert(20);
    tree.insert(10);
    tree.insert(30);
    ASSERT_EQ(tree.height(), 1u);  // Still single node

    // Insert one more to trigger split
    tree.insert(25);
    ASSERT_TRUE(tree.height() >= 1u);  // May have split

    // Verify all elements present
    ASSERT_TRUE(tree.search(10));
    ASSERT_TRUE(tree.search(20));
    ASSERT_TRUE(tree.search(25));
    ASSERT_TRUE(tree.search(30));
    ASSERT_EQ(tree.size(), 4u);

    // Verify sorted iteration
    std::vector<int> result;
    for (const auto& v : tree) {
        result.push_back(v);
    }
    std::vector<int> expected = {10, 20, 25, 30};
    ASSERT_TRUE(result == expected);
}

// === Tier 2: High Priority Edge Cases ===

// Test: fill_child boundary - child has exactly min_keys
TEST(test_fill_child_exact_min_keys) {
    BTree<int, 4> tree;  // Order 4: min_keys=1

    // Insert elements
    for (int i = 1; i <= 20; i++) {
        tree.insert(i);
    }

    // Remove elements to create nodes with exactly min_keys
    // Then remove more to trigger fill_child
    for (int i = 1; i <= 10; i++) {
        ASSERT_TRUE(tree.remove(i));
        // After each remove, tree should remain valid
        for (int j = i + 1; j <= 20; j++) {
            ASSERT_TRUE(tree.search(j));
        }
    }

    ASSERT_EQ(tree.size(), 10u);
}

// Test: Borrow from left when left has exactly min_keys + 1
TEST(test_borrow_left_boundary) {
    BTree<int, 5> tree;  // Order 5: min_keys=2

    // Build tree
    for (int i = 1; i <= 30; i++) {
        tree.insert(i);
    }

    // Remove from right side progressively
    for (int i = 30; i >= 20; i--) {
        ASSERT_TRUE(tree.remove(i));
    }

    // Verify remaining
    for (int i = 1; i <= 19; i++) {
        ASSERT_TRUE(tree.search(i));
    }
}

// Test: Borrow from right when right has exactly min_keys + 1
TEST(test_borrow_right_boundary) {
    BTree<int, 5> tree;  // Order 5: min_keys=2

    // Build tree
    for (int i = 1; i <= 30; i++) {
        tree.insert(i);
    }

    // Remove from left side progressively
    for (int i = 1; i <= 11; i++) {
        ASSERT_TRUE(tree.remove(i));
    }

    // Verify remaining
    for (int i = 12; i <= 30; i++) {
        ASSERT_TRUE(tree.search(i));
    }
}

// Test: Post-merge parent doesn't overflow
TEST(test_merge_parent_no_overflow) {
    BTree<int, 3> tree;  // Order 3: tightest constraints

    // Insert enough to create multi-level tree
    for (int i = 1; i <= 30; i++) {
        tree.insert(i);
    }

    // Remove all elements one by one
    // This tests many merge scenarios
    for (int i = 1; i <= 30; i++) {
        ASSERT_TRUE(tree.remove(i));

        // After each remove, iteration should work (tree valid)
        int count = 0;
        for (const auto& v : tree) {
            (void)v;
            count++;
        }
        ASSERT_EQ(static_cast<size_t>(count), tree.size());
    }

    ASSERT_TRUE(tree.empty());
}

// Test: Height reduction at exact moment
TEST(test_height_reduction_exact_moment) {
    BTree<int, 4> tree;

    // Build tree to specific height
    for (int i = 1; i <= 50; i++) {
        tree.insert(i);
    }

    size_t initial_height = tree.height();
    ASSERT_TRUE(initial_height >= 2u);

    // Track height changes during removal
    size_t prev_height = initial_height;
    bool height_reduced = false;

    for (int i = 1; i <= 45; i++) {
        tree.remove(i);
        size_t curr_height = tree.height();

        if (curr_height < prev_height) {
            height_reduced = true;
            // Verify tree still valid at height reduction point
            for (int j = i + 1; j <= 50; j++) {
                ASSERT_TRUE(tree.search(j));
            }
        }
        prev_height = curr_height;
    }

    // Height should have reduced at some point
    ASSERT_TRUE(height_reduced || tree.height() < initial_height);
}

// Test: Insert causing cascade splits (Order 3)
TEST(test_insert_cascade_splits) {
    BTree<int, 3> tree;  // Order 3: max=2, frequent splits

    // Track height increases
    size_t heights_seen = 0;
    size_t last_height = 0;

    for (int i = 1; i <= 50; i++) {
        tree.insert(i);
        size_t h = tree.height();
        if (h > last_height) {
            heights_seen++;
            last_height = h;
        }
    }

    // Should have seen multiple height increases (cascade splits)
    ASSERT_TRUE(heights_seen >= 3u);

    // Verify all elements present
    for (int i = 1; i <= 50; i++) {
        ASSERT_TRUE(tree.search(i));
    }
}

// Test: Merge where mid calculation ensures valid split
TEST(test_merge_split_mid_calculation) {
    BTree<int, 3> tree;  // Order 3

    // Insert and remove in pattern that triggers merge-split
    for (int i = 1; i <= 20; i++) {
        tree.insert(i);
    }

    // Remove in pattern that forces merges
    std::vector<int> remove_order = {10, 5, 15, 3, 8, 12, 18};
    for (int val : remove_order) {
        if (tree.search(val)) {
            ASSERT_TRUE(tree.remove(val));
        }
    }

    // Tree should still be valid - verify via iteration
    std::vector<int> remaining;
    for (const auto& v : tree) {
        remaining.push_back(v);
    }

    // Verify sorted
    for (size_t i = 1; i < remaining.size(); i++) {
        ASSERT_TRUE(remaining[i] > remaining[i-1]);
    }
}

// Test: Index logic after fill_child (is_last flag)
TEST(test_fill_child_is_last_flag) {
    BTree<int, 4> tree;

    // Insert elements
    for (int i = 1; i <= 25; i++) {
        tree.insert(i);
    }

    // Remove last elements to trigger is_last scenarios
    for (int i = 25; i >= 15; i--) {
        ASSERT_TRUE(tree.remove(i));

        // Verify tree integrity after each removal
        for (int j = 1; j < i; j++) {
            ASSERT_TRUE(tree.search(j));
        }
    }

    ASSERT_EQ(tree.size(), 14u);
}

// === Tier 3: Medium Priority Edge Cases ===

// Test: find() returns valid iterator for duplicates that can be incremented
TEST(test_find_duplicate_iterator_validity) {
    BTree<int> tree;

    tree.insert(10);
    tree.insert(20);
    tree.insert(10);  // Duplicate
    tree.insert(30);
    tree.insert(10);  // Another duplicate

    auto it = tree.find(10);
    ASSERT_TRUE(it != tree.end());
    ASSERT_EQ(*it, 10);

    // Iterator should be incrementable
    ++it;
    // Next element should also be valid
    ASSERT_TRUE(it != tree.end());

    // Count all elements via iteration from found position
    int count = 1;  // Already at first 10
    while (it != tree.end()) {
        count++;
        ++it;
    }
    ASSERT_TRUE(count >= 1);  // At least found the element
}

// Test: Cross-tree iterator comparison
TEST(test_iterator_cross_tree_comparison) {
    BTree<int> tree1;
    BTree<int> tree2;

    tree1.insert(1);
    tree2.insert(2);

    // End iterators from same tree should be equal
    ASSERT_TRUE(tree1.end() == tree1.end());
    ASSERT_TRUE(tree2.end() == tree2.end());

    // Begin iterators from different trees should not be equal
    // (they point to different elements)
    ASSERT_TRUE(*tree1.begin() != *tree2.begin());
}

// Test: Predecessor retrieval on various tree shapes
TEST(test_get_predecessor_shapes) {
    // Test min() which uses leftmost traversal (similar to predecessor logic)
    BTree<int, 4> tree;

    // Linear insertion
    for (int i = 10; i >= 1; i--) {
        tree.insert(i);
        ASSERT_EQ(tree.min(), i);  // Min should update
    }

    // Random insertion maintaining min
    tree.clear();
    tree.insert(50);
    ASSERT_EQ(tree.min(), 50);

    tree.insert(25);
    ASSERT_EQ(tree.min(), 25);

    tree.insert(75);
    ASSERT_EQ(tree.min(), 25);  // Unchanged

    tree.insert(10);
    ASSERT_EQ(tree.min(), 10);

    tree.insert(5);
    ASSERT_EQ(tree.min(), 5);
}

// Test: Successor retrieval on various tree shapes
TEST(test_get_successor_shapes) {
    // Test max() which uses rightmost traversal (similar to successor logic)
    BTree<int, 4> tree;

    // Linear insertion
    for (int i = 1; i <= 10; i++) {
        tree.insert(i);
        ASSERT_EQ(tree.max(), i);  // Max should update
    }

    // Random insertion maintaining max
    tree.clear();
    tree.insert(50);
    ASSERT_EQ(tree.max(), 50);

    tree.insert(75);
    ASSERT_EQ(tree.max(), 75);

    tree.insert(25);
    ASSERT_EQ(tree.max(), 75);  // Unchanged

    tree.insert(90);
    ASSERT_EQ(tree.max(), 90);

    tree.insert(100);
    ASSERT_EQ(tree.max(), 100);
}

// Test: Height stability when it shouldn't change
TEST(test_height_stable_after_removes) {
    BTree<int, 10> tree;  // High order = more stable height

    // Insert many elements
    for (int i = 1; i <= 100; i++) {
        tree.insert(i);
    }

    size_t initial_height = tree.height();

    // Remove a few elements - height should stay stable
    tree.remove(50);
    tree.remove(51);
    tree.remove(52);

    // Height likely unchanged for high-order tree with few removals
    ASSERT_TRUE(tree.height() <= initial_height);

    // Verify remaining elements
    for (int i = 1; i <= 100; i++) {
        if (i >= 50 && i <= 52) {
            ASSERT_FALSE(tree.search(i));
        } else {
            ASSERT_TRUE(tree.search(i));
        }
    }
}

// Test: Order 5 with explicit min_keys=2 boundary testing
TEST(test_order_5_min_keys_boundary) {
    BTree<int, 5> tree;  // Order 5: min_keys=2, max_keys=4

    // Insert elements
    for (int i = 1; i <= 40; i++) {
        tree.insert(i);
    }

    // Remove in pattern that tests min_keys=2 boundary
    // Remove every 3rd element
    for (int i = 3; i <= 40; i += 3) {
        ASSERT_TRUE(tree.remove(i));
    }

    // Verify remaining elements
    for (int i = 1; i <= 40; i++) {
        if (i % 3 == 0) {
            ASSERT_FALSE(tree.search(i));
        } else {
            ASSERT_TRUE(tree.search(i));
        }
    }

    // Verify tree integrity via iteration
    std::vector<int> result;
    for (const auto& v : tree) {
        result.push_back(v);
    }

    // Should be sorted
    for (size_t i = 1; i < result.size(); i++) {
        ASSERT_TRUE(result[i] > result[i-1]);
    }
}

// === Duplicate-heavy differential fuzz against std::multiset ===
//
// The existing stress tests (test_random_operations_integrity, etc.) compare
// against std::set and deliberately insert only UNIQUE values "to avoid
// duplicate complexity". But duplicates are exactly where the tree's subtle
// contracts live: upper_index places a new duplicate after existing equals,
// split medians can be duplicated keys, remove_rec's predecessor swap can have
// predecessor == separator, and find() must return the LEFTMOST equal element.
// This mirrors a BTree against a std::multiset through a random insert/remove
// workload with a small key range (so keys collide heavily) and asserts, at
// every step, that size and membership agree and that the full in-order contents
// match element-for-element (multiplicities included). A final randomized drain
// removes every element, exercising borrow/merge on duplicate-laden nodes.
template <int Order>
static void multiset_differential(unsigned seed, int range, int ops) {
    BTree<int, Order> tree;
    std::multiset<int> ref;
    std::mt19937 rng(seed);

    for (int step = 0; step < ops; ++step) {
        int key = static_cast<int>(rng() % static_cast<unsigned>(range));
        bool do_insert = (rng() % 100u) < 55u;  // growth-biased: builds up deep
                                                 // stacks of duplicates over time

        if (do_insert) {
            tree.insert(key);
            ref.insert(key);
        } else {
            bool tree_removed = tree.remove(key);
            auto rit = ref.find(key);
            bool ref_removed = (rit != ref.end());
            if (ref_removed) ref.erase(rit);  // erase exactly ONE instance, not all
            ASSERT_EQ(tree_removed, ref_removed);
        }

        // Cheap invariants checked every step.
        ASSERT_EQ(tree.size(), ref.size());
        ASSERT_EQ(tree.search(key), ref.count(key) > 0);
        ASSERT_EQ(tree.empty(), ref.empty());

        // Expensive full-contents check amortized over every 16th step (+ last).
        if ((step & 15) == 0 || step == ops - 1) {
            std::vector<int> want(ref.begin(), ref.end());  // multiset: sorted
            ASSERT_TRUE(tree.to_vector() == want);          // to_vector path
            std::vector<int> via_iter;                      // range-for path
            for (int v : tree) via_iter.push_back(v);
            ASSERT_TRUE(via_iter == want);
            if (!ref.empty()) {
                ASSERT_EQ(tree.min(), *ref.begin());
                ASSERT_EQ(tree.max(), *ref.rbegin());
            }
        }
    }

    // Drain every element in a shuffled order, staying in lockstep with the
    // reference. Each remove() must succeed (the key came from ref) and drop
    // size by exactly one; this walks the tree back down through every
    // borrow/merge shape with duplicates still present.
    std::vector<int> remaining(ref.begin(), ref.end());
    std::shuffle(remaining.begin(), remaining.end(), rng);
    for (int k : remaining) {
        ASSERT_TRUE(tree.remove(k));
        auto rit = ref.find(k);
        ASSERT_TRUE(rit != ref.end());
        ref.erase(rit);
        ASSERT_EQ(tree.size(), ref.size());
    }
    ASSERT_TRUE(tree.empty());
    ASSERT_EQ(tree.size(), 0u);
    ASSERT_EQ(tree.height(), 0u);
}

TEST(test_multiset_differential_stress) {
    multiset_differential<3>(1u, 32, 2500);    // order 3: min fan-out, tall trees
    multiset_differential<4>(2u, 40, 2500);
    multiset_differential<7>(3u, 64, 3000);
    multiset_differential<64>(4u, 200, 4000);  // default order, wider key range
}

// find() must return the FIRST (leftmost, in-order) equal element even when a
// key's duplicates straddle an internal separator and its left subtree. This
// pins that contract precisely: for every probed key, [find(k), end()) must
// equal the multiset suffix [lower_bound(k), end()) -- i.e. iterating from
// find(k) reproduces the entire sorted tail beginning at the first occurrence.
// A naive descent that stopped at the first internal match would yield a middle
// occurrence and silently drop the earlier duplicates, breaking this equality.
TEST(test_find_halfopen_range_duplicates) {
    BTree<int, 3> tree;       // small order => many internal levels hold separators
    std::multiset<int> ref;
    std::mt19937 rng(20260707u);
    const int K = 60;
    for (int v = 0; v <= K; ++v) {
        int copies = static_cast<int>(rng() % 5u);  // 0..4 copies (some keys absent)
        for (int c = 0; c < copies; ++c) {
            tree.insert(v);
            ref.insert(v);
        }
    }

    for (int k = -2; k <= K + 2; ++k) {  // probe present AND absent keys, incl. ends
        auto it = tree.find(k);
        bool present = ref.count(k) > 0;
        if (!present) {
            ASSERT_TRUE(it == tree.end());
            continue;
        }
        ASSERT_TRUE(it != tree.end());
        ASSERT_EQ(*it, k);  // iterator points at the value itself

        std::vector<int> got;
        for (auto j = it; j != tree.end(); ++j) got.push_back(*j);
        std::vector<int> want(ref.lower_bound(k), ref.end());
        ASSERT_TRUE(got == want);
    }
}

// A moved-from POPULATED tree must remain a usable, empty tree. Its SlabPools
// hand their real (heap-backed) slabs to the destination but retain their
// size/align configuration, so the source can allocate fresh slabs and be
// filled again from scratch. (test_move_empty_tree only moves an already-empty
// source, and test_self_move_assignment is a no-op self-move -- neither drives
// re-population of a genuinely emptied-out pool.)
TEST(test_reuse_populated_tree_after_move) {
    BTree<int, 4> src;
    for (int i = 0; i < 200; ++i) src.insert(i);
    ASSERT_EQ(src.size(), 200u);

    // Move-construct: contents transfer wholesale to dst.
    BTree<int, 4> dst(std::move(src));
    ASSERT_EQ(dst.size(), 200u);
    for (int i = 0; i < 200; ++i) ASSERT_TRUE(dst.search(i));

    // Source is now empty AND reusable with a fresh, disjoint key set.
    ASSERT_TRUE(src.empty());
    ASSERT_EQ(src.size(), 0u);
    ASSERT_EQ(src.height(), 0u);
    for (int i = 1000; i < 1300; ++i) src.insert(i);
    ASSERT_EQ(src.size(), 300u);
    for (int i = 1000; i < 1300; ++i) ASSERT_TRUE(src.search(i));
    ASSERT_FALSE(src.search(0));  // never received the moved-away contents

    // Move-assign onto a populated destination: dst frees its own nodes, adopts
    // src2's storage, and src2 is likewise left reusable.
    BTree<int, 4> src2;
    for (int i = 0; i < 150; ++i) src2.insert(i * 2);
    dst = std::move(src2);
    ASSERT_EQ(dst.size(), 150u);
    for (int i = 0; i < 150; ++i) ASSERT_TRUE(dst.search(i * 2));
    ASSERT_FALSE(dst.search(1));  // dst's prior contents were released
    ASSERT_TRUE(src2.empty());
    src2.insert(999);
    ASSERT_EQ(src2.size(), 1u);
    ASSERT_TRUE(src2.search(999));
}

// A tree built entirely from ONE repeated key: every split median is that key
// and every internal separator equals it, so a tall order-3 tree has the same
// value at every level. Removing the copies one at a time drives remove_rec's
// predecessor swap with predecessor == separator == key at each level, plus
// underflow repair (borrow/merge) across all-identical nodes. Size must fall by
// exactly one per removal and the key stay findable until the last copy is gone.
TEST(test_all_identical_insert_remove) {
    const int M = 400;
    BTree<int, 3> tree;
    for (int i = 0; i < M; ++i) tree.insert(7);
    ASSERT_EQ(tree.size(), static_cast<size_t>(M));
    ASSERT_TRUE(tree.search(7));
    ASSERT_EQ(tree.min(), 7);
    ASSERT_EQ(tree.max(), 7);
    ASSERT_TRUE(tree.height() > 1u);  // genuinely multi-level, not a single leaf

    long count = 0;
    for (int v : tree) { ASSERT_EQ(v, 7); ++count; }
    ASSERT_EQ(count, static_cast<long>(M));

    for (int i = 0; i < M; ++i) {
        ASSERT_TRUE(tree.search(7));
        ASSERT_TRUE(tree.remove(7));
        ASSERT_EQ(tree.size(), static_cast<size_t>(M - 1 - i));
    }
    ASSERT_TRUE(tree.empty());
    ASSERT_FALSE(tree.search(7));
    ASSERT_FALSE(tree.remove(7));  // removing from an empty tree is a no-op false
}

// === remove() strong exception-safety ===

// Key type whose COPY can be made to throw on demand, but whose MOVE is noexcept
// (as BTree requires). `copy_budget` throws once that many copies have happened
// since the last reset; `live` tracks net instances for leak detection.
struct Thrower {
    int v;
    static long live;
    static long copies;
    static long copy_budget;  // -1 disables throwing

    Thrower(int val = 0) : v(val) { ++live; }
    Thrower(const Thrower& o) : v(o.v) {
        if (copy_budget >= 0 && copies >= copy_budget) {
            throw std::runtime_error("Thrower copy ctor");
        }
        ++copies;
        ++live;
    }
    Thrower& operator=(const Thrower& o) {
        if (copy_budget >= 0 && copies >= copy_budget) {
            throw std::runtime_error("Thrower copy assign");
        }
        ++copies;
        v = o.v;
        return *this;
    }
    Thrower(Thrower&& o) noexcept : v(o.v) { ++live; }
    Thrower& operator=(Thrower&& o) noexcept { v = o.v; return *this; }
    ~Thrower() { --live; }
    bool operator<(const Thrower& o) const noexcept { return v < o.v; }
    bool operator==(const Thrower& o) const noexcept { return v == o.v; }
};
long Thrower::live = 0;
long Thrower::copies = 0;
long Thrower::copy_budget = -1;

// Walk a tree down to empty; before each real removal, sweep the copy budget so a
// copy throws at every point remove() performs one, and assert the STRONG
// guarantee each time (tree byte-for-byte unchanged, size consistent, every key
// still present). Returns the number of throws actually exercised.
template <int Order>
static long remove_exc_safety_walk(unsigned seed) {
    Thrower::copy_budget = -1;
    Thrower::copies = 0;
    long throws_seen = 0;

    BTree<Thrower, Order> tree;
    std::multiset<int> ref;
    std::mt19937 rng(seed);
    for (int i = 0; i < 80; ++i) {
        int x = static_cast<int>(rng() % 40);
        tree.insert(Thrower(x));
        ref.insert(x);
    }

    while (!ref.empty()) {
        std::vector<int> snap(ref.begin(), ref.end());
        size_t n = tree.size();
        int key = snap[rng() % snap.size()];

        for (long k = 0;; ++k) {
            if (k > 500) throw std::runtime_error("budget sweep did not converge");
            Thrower::copies = 0;
            Thrower::copy_budget = k;
            bool threw = false;
            try {
                tree.remove(Thrower(key));
            } catch (const std::exception&) {
                threw = true;
            }
            Thrower::copy_budget = -1;  // disable throwing for the assertions below

            if (threw) {
                ++throws_seen;
                // Strong guarantee: nothing changed.
                ASSERT_EQ(tree.size(), n);
                std::vector<int> now;
                now.reserve(n);
                for (const auto& t : tree) now.push_back(t.v);
                ASSERT_TRUE(now == snap);
                // No separator was duplicated / dropped: every key still findable.
                for (int kv : snap) ASSERT_TRUE(tree.find(Thrower(kv)) != tree.end());
            } else {
                // Removed exactly one occurrence.
                ref.erase(ref.find(key));
                ASSERT_EQ(tree.size(), ref.size());
                std::vector<int> now;
                now.reserve(ref.size());
                for (const auto& t : tree) now.push_back(t.v);
                std::vector<int> expected(ref.begin(), ref.end());
                ASSERT_TRUE(now == expected);
                break;
            }
        }
    }
    return throws_seen;
}

TEST(test_remove_strong_exception_safety) {
    Thrower::live = 0;
    long total_throws = 0;
    total_throws += remove_exc_safety_walk<3>(1);
    total_throws += remove_exc_safety_walk<3>(2);
    total_throws += remove_exc_safety_walk<4>(3);
    total_throws += remove_exc_safety_walk<5>(4);
    total_throws += remove_exc_safety_walk<6>(5);
    total_throws += remove_exc_safety_walk<64>(6);

    // The strong-guarantee path must actually have been exercised (otherwise the
    // test would pass vacuously if remove() stopped copying entirely).
    ASSERT_TRUE(total_throws > 0);
    // Every Thrower constructed was destroyed: no leak on any throwing path.
    ASSERT_EQ(Thrower::live, 0L);
}

// Deterministic, readable companion to the randomized walk: repeatedly remove the
// minimum of an Order-3 tree, which forces the leftmost leaf to underflow and
// repair via borrow_from_next / merge. Sweeping the copy budget makes a copy throw
// at every point the repair would perform one; the tree must be left exactly
// intact each time. This is precisely where the pre-fix code duplicated/dropped a
// separator when a borrow's second copy threw, so this case fails pre-fix.
TEST(test_remove_rebalance_throw_strong) {
    Thrower::live = 0;
    Thrower::copy_budget = -1;
    Thrower::copies = 0;
    {
        BTree<Thrower, 3> tree;
        std::vector<int> present;
        for (int i = 0; i < 24; ++i) {
            tree.insert(Thrower(i));
            present.push_back(i);
        }

        while (!present.empty()) {
            std::vector<int> snap = present;
            size_t n = tree.size();
            int key = present.front();  // remove the minimum
            for (long k = 0;; ++k) {
                if (k > 500) throw std::runtime_error("budget sweep did not converge");
                Thrower::copies = 0;
                Thrower::copy_budget = k;
                bool threw = false;
                try {
                    tree.remove(Thrower(key));
                } catch (const std::exception&) {
                    threw = true;
                }
                Thrower::copy_budget = -1;
                if (threw) {
                    ASSERT_EQ(tree.size(), n);
                    std::vector<int> now;
                    for (const auto& t : tree) now.push_back(t.v);
                    ASSERT_TRUE(now == snap);
                } else {
                    present.erase(present.begin());
                    ASSERT_EQ(tree.size(), present.size());
                    break;
                }
            }
        }
    }
    ASSERT_EQ(Thrower::live, 0L);  // no leak on any throwing path
}

// === insert() strong exception-safety under allocation failure ===
//
// insert() commits the key into a leaf and then splits any overflowed nodes on
// the recursion unwind, and those splits allocate nodes. Historically, if a
// split's make_node() threw std::bad_alloc *after* the key was committed, the
// tree was left with an over-capacity node (max_keys+1 keys) and an uncounted
// size_ -- a corrupt state that a later insert would turn into an out-of-bounds
// write past a node's fixed InlineVec storage. insert() now pre-reserves the
// entire split cascade before committing, so an allocation failure can only
// strike at that pre-commit reservation and leaves the tree exactly as it was.
//
// This test injects bad_alloc at *every* allocation point reached during a build
// and, for each, asserts the strong guarantee: the tree holds exactly the keys
// that were successfully inserted before the failing insert, with a consistent
// size(), and remains fully usable afterwards (finishing the build yields the
// complete, correct set with no latent corruption).

// Fail-injection hook on the ALIGNED operator new only. That is exactly (and
// only) what SlabPool uses for node slabs -- it calls ::operator new(bytes,
// align_val_t) explicitly -- so this intercepts every BTree heap allocation
// while touching nothing else in the program. We deliberately do NOT replace the
// ordinary operator new/delete: std::string / std::stringstream buffers can be
// allocated inside the shared libstdc++ runtime and then freed via a delete that
// -O2 inlines into this TU, mixing two heaps and corrupting on free. char and
// other max-aligned types never use the aligned new, so the aligned-only hook
// sidesteps that entirely. The pool's aligned new/delete are both emitted into
// this TU (btree.hpp is header-only), so they pair consistently.
//
// While armed, the g_alloc_countdown-th slab allocation throws bad_alloc;
// otherwise it is a malloc pass-through. (g_alloc_armed has static/zero init, so
// it is false before any dynamic initialization that might allocate.) std::malloc
// returns max_align_t (>=16 byte) storage, which covers every node alignment the
// suite uses (int/double/string/Thrower all need <= 16); an over-aligned key type
// would need real aligned allocation here.
static bool g_alloc_armed = false;
static long g_alloc_countdown = 0;
static long g_alloc_hits = 0;  // armed slab allocations seen (whether or not they threw)

static void* test_slab_alloc(std::size_t n, std::size_t align) {
    (void)align;  // see note above: max_align_t storage suffices for this suite
    if (g_alloc_armed) {
        ++g_alloc_hits;
        if (--g_alloc_countdown < 0) throw std::bad_alloc();
    }
    void* p = std::malloc(n ? n : 1);
    if (!p) throw std::bad_alloc();
    return p;
}
void* operator new(std::size_t n, std::align_val_t a) {
    return test_slab_alloc(n, static_cast<std::size_t>(a));
}
void* operator new[](std::size_t n, std::align_val_t a) {
    return test_slab_alloc(n, static_cast<std::size_t>(a));
}
void operator delete(void* p, std::align_val_t) noexcept { std::free(p); }
void operator delete[](void* p, std::align_val_t) noexcept { std::free(p); }
void operator delete(void* p, std::size_t, std::align_val_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t, std::align_val_t) noexcept { std::free(p); }

// RAII arm/disarm so the hook is ALWAYS disarmed on scope exit -- even if an
// unexpected exception unwinds through the armed region -- and can never leak
// its armed state into a later test.
struct AllocFailGuard {
    explicit AllocFailGuard(long countdown) {
        g_alloc_hits = 0;
        g_alloc_countdown = countdown;
        g_alloc_armed = true;
    }
    ~AllocFailGuard() { g_alloc_armed = false; }
    AllocFailGuard(const AllocFailGuard&) = delete;
    AllocFailGuard& operator=(const AllocFailGuard&) = delete;
};

// Build 0..M-1 into a fresh Order tree, injecting a bad_alloc at the `target`-th
// allocation; verify the strong guarantee holds after the throw. Sweeps `target`
// over every allocation the build performs. Returns the number of injected
// throws actually exercised (so the caller can assert coverage was non-vacuous).
template <int Order>
static long insert_alloc_fail_sweep(int M) {
    // First, an un-injected armed build to count how many allocation points exist.
    // LONG_MAX countdown never reaches zero, so this build cannot throw.
    long total_allocs;
    {
        BTree<int, Order> t;
        AllocFailGuard guard(LONG_MAX);
        for (int k = 0; k < M; ++k) t.insert(k);
        total_allocs = g_alloc_hits;
    }

    long throws_seen = 0;
    for (long target = 0; target <= total_allocs; ++target) {
        BTree<int, Order> t;
        int succeeded = 0;
        bool threw = false;

        {
            AllocFailGuard guard(target);
            for (int k = 0; k < M; ++k) {
                try {
                    t.insert(k);
                    ++succeeded;
                } catch (const std::bad_alloc&) {
                    threw = true;
                    break;
                }
            }
        }  // guard disarms the hook here, before any allocating assertion below
        if (threw) ++throws_seen;

        // Strong guarantee: exactly {0 .. succeeded-1}, in order, size() consistent.
        // (Pre-fix, a split that threw left an extra committed-but-uncounted key,
        // so the in-order element count would exceed size().)
        ASSERT_EQ(t.size(), static_cast<size_t>(succeeded));
        long iterated = 0;
        for (int v : t) {
            ASSERT_EQ(v, static_cast<int>(iterated));
            ++iterated;
        }
        ASSERT_EQ(iterated, static_cast<long>(succeeded));

        // No latent corruption: finish the build with the allocator healthy and
        // confirm the full, correct set (pre-fix this path overran a node buffer).
        for (int k = succeeded; k < M; ++k) t.insert(k);
        ASSERT_EQ(t.size(), static_cast<size_t>(M));
        int expect = 0;
        for (int v : t) { ASSERT_EQ(v, expect); ++expect; }
        ASSERT_EQ(expect, M);
    }
    return throws_seen;
}

TEST(test_insert_strong_exception_safety_alloc_fail) {
    long throws = 0;
    throws += insert_alloc_fail_sweep<3>(300);
    throws += insert_alloc_fail_sweep<4>(300);
    throws += insert_alloc_fail_sweep<5>(300);
    throws += insert_alloc_fail_sweep<64>(800);
    throws += insert_alloc_fail_sweep<3>(6000);  // tall tree: inject across slab
                                                 // boundaries reached mid-build
    // The injected-failure path must actually have fired (else the test would
    // pass vacuously if insert() stopped allocating during splits).
    ASSERT_TRUE(throws > 0);
    // The hook must be disarmed so it cannot perturb any later test.
    ASSERT_FALSE(g_alloc_armed);
}

// === Move-aware insert(): rvalue keys are relocated, not copied ===
//
// A key type that separately tallies copy-constructions/assignments and
// move-constructions/assignments, so a test can prove that inserting an rvalue
// key MOVES it into the leaf rather than copying it. Move ops are noexcept (as
// BTree requires); nothing here throws.
struct MoveCounter {
    int v;
    static long copies;  // copy ctor + copy assign
    static long moves;   // move ctor + move assign

    MoveCounter(int val = 0) : v(val) {}
    MoveCounter(const MoveCounter& o) : v(o.v) { ++copies; }
    MoveCounter& operator=(const MoveCounter& o) { v = o.v; ++copies; return *this; }
    MoveCounter(MoveCounter&& o) noexcept : v(o.v) { ++moves; }
    MoveCounter& operator=(MoveCounter&& o) noexcept { v = o.v; ++moves; return *this; }
    bool operator<(const MoveCounter& o) const noexcept { return v < o.v; }
    bool operator==(const MoveCounter& o) const noexcept { return v == o.v; }
};
long MoveCounter::copies = 0;
long MoveCounter::moves = 0;

// Inserting rvalue keys must never copy the key: each key is moved from the
// caller's temporary, down the descent, and into its leaf slot. Pre-change the
// rvalue bound to insert(const T&) and every key was copy-constructed into the
// leaf (copies == number of inserts); the move overload makes copies == 0.
TEST(test_insert_rvalue_moves_not_copies) {
    MoveCounter::copies = 0;
    MoveCounter::moves = 0;

    // Small order so the build cascades through many splits (all move-only),
    // exercising the descent + leaf placement + split relocation paths.
    BTree<MoveCounter, 6> tree;
    const int N = 300;
    for (int i = 0; i < N; ++i) {
        tree.insert(MoveCounter(i * 7 % N));  // rvalue temporary
    }

    ASSERT_EQ(MoveCounter::copies, 0L);   // no key was ever copied
    ASSERT_TRUE(MoveCounter::moves > 0);  // keys were relocated by move

    // Correctness is unchanged: every key is present and the tree is well-formed.
    ASSERT_EQ(tree.size(), static_cast<size_t>(N));
    for (int i = 0; i < N; ++i) {
        ASSERT_TRUE(tree.search(MoveCounter(i)));
    }
    std::vector<int> in_order;
    for (const auto& mc : tree) in_order.push_back(mc.v);
    ASSERT_EQ(in_order.size(), static_cast<size_t>(N));
    for (int i = 0; i < N; ++i) ASSERT_EQ(in_order[i], i);
}

// The lvalue overload must still copy: it may not steal from the caller's
// object. Guards against a careless refactor that forwards an lvalue as an
// rvalue and leaves the source moved-from.
TEST(test_insert_lvalue_preserves_source) {
    BTree<std::string, 8> tree;
    std::string s = "persistent-source-string";
    tree.insert(s);  // lvalue: must copy, leaving s intact
    ASSERT_EQ(s, std::string("persistent-source-string"));
    ASSERT_TRUE(tree.search(std::string("persistent-source-string")));

    // A moved-in rvalue string must also land correctly (value-level check).
    std::string t = "moved-in-string";
    tree.insert(std::move(t));
    ASSERT_TRUE(tree.search(std::string("moved-in-string")));
}

// === Bulk-load range constructor: BTree(first, last) / initializer_list ===
//
// A bulk-loaded tree must be a fully valid B-tree: the same multiset of keys in
// sorted in-order, every key findable, and -- crucially -- structurally sound
// enough that a full teardown-by-remove of every element succeeds (which only
// holds if every non-root node was built within [min_keys, max_keys]). We can't
// see node fill factors through the public API, so removing every element and
// confirming the tree empties cleanly is the structural-validity proxy.
template <int Order>
static void bulk_load_check(const std::vector<int>& input) {
    std::vector<int> sorted = input;
    std::sort(sorted.begin(), sorted.end());

    BTree<int, Order> tree(input.begin(), input.end());

    ASSERT_EQ(tree.size(), input.size());

    std::vector<int> got;
    for (int v : tree) got.push_back(v);
    ASSERT_TRUE(got == sorted);  // exact multiset + order match

    for (int v : input) ASSERT_TRUE(tree.search(v));

    if (input.empty()) {
        ASSERT_TRUE(tree.empty());
        ASSERT_EQ(tree.height(), static_cast<size_t>(0));
    } else {
        ASSERT_TRUE(tree.height() >= 1);
        // Height must be genuinely logarithmic (a balanced build), not a
        // degenerate chain: comfortably bounded by 2*log_{min+1}(n) + 2.
        size_t branch = static_cast<size_t>((Order - 1) / 2) + 1;  // min_keys+1
        if (branch < 2) branch = 2;
        size_t bound = 2;
        size_t cap = 1;
        while (cap < input.size()) { cap *= branch; ++bound; }
        ASSERT_TRUE(tree.height() <= bound + 2);
    }

    // Structural soundness: removing every element (each occurrence once) must
    // succeed and leave the tree empty, exercising borrow/merge over the built
    // shape. Remove in a shuffled order to stress rebalancing paths.
    std::vector<int> order = input;
    std::mt19937 rng(99);
    std::shuffle(order.begin(), order.end(), rng);
    for (int v : order) ASSERT_TRUE(tree.remove(v));
    ASSERT_TRUE(tree.empty());
    ASSERT_EQ(tree.size(), static_cast<size_t>(0));
}

// Bulk load must produce a correct, balanced tree across many sizes and orders,
// including the boundary sizes (0,1,2,3) and the smallest order (3), and for
// random, already-sorted, and duplicate-heavy inputs. Fails to compile before
// the BTree(first,last) constructor exists; passes once it does.
TEST(test_bulk_load_range_constructor) {
    std::mt19937 rng(2024);
    for (size_t n : {size_t(0), size_t(1), size_t(2), size_t(3), size_t(4),
                     size_t(5), size_t(7), size_t(10), size_t(50), size_t(100),
                     size_t(257), size_t(1000), size_t(5000)}) {
        std::vector<int> data(n);
        for (size_t i = 0; i < n; ++i) data[i] = static_cast<int>(rng() % 100000);
        bulk_load_check<3>(data);
        bulk_load_check<4>(data);
        bulk_load_check<5>(data);
        bulk_load_check<7>(data);
        bulk_load_check<64>(data);
        bulk_load_check<100>(data);

        std::vector<int> s = data;
        std::sort(s.begin(), s.end());
        bulk_load_check<3>(s);
        bulk_load_check<64>(s);

        std::vector<int> dup(n);
        for (size_t i = 0; i < n; ++i) dup[i] = static_cast<int>(rng() % 5);
        bulk_load_check<3>(dup);
        bulk_load_check<6>(dup);
        bulk_load_check<64>(dup);
    }
}

// The initializer_list constructor sorts and de-interleaves duplicates.
TEST(test_bulk_load_initializer_list) {
    BTree<int, 5> t{5, 3, 9, 1, 7, 3, 8, 2};
    std::vector<int> got;
    for (int v : t) got.push_back(v);
    std::vector<int> exp{1, 2, 3, 3, 5, 7, 8, 9};
    ASSERT_TRUE(got == exp);
    ASSERT_EQ(t.size(), static_cast<size_t>(8));
    ASSERT_TRUE(t.find(3) != t.end());
    ASSERT_FALSE(t.search(4));
}

// find() on a bulk-loaded tree must still return the LEFTMOST duplicate: a
// separator key equal to values in an adjacent leaf must not shadow them.
TEST(test_bulk_load_find_leftmost_duplicate) {
    std::vector<int> data;
    for (int i = 0; i < 200; ++i) data.push_back(i % 10);  // 20 each of 0..9
    BTree<int, 4> t(data.begin(), data.end());
    for (int k = 0; k < 10; ++k) {
        auto it = t.find(k);
        ASSERT_TRUE(it != t.end());
        int count = 0;
        while (it != t.end() && *it == k) { ++count; ++it; }
        ASSERT_EQ(count, 20);  // saw every occurrence starting from the first
    }
}

// === Range queries: lower_bound / upper_bound / equal_range / count ===
//
// Differential check against std::multiset, which defines the exact semantics
// we must match: lower_bound = first element >= key, upper_bound = first > key,
// equal_range = [lower, upper), count = number of occurrences. Covers present
// and absent keys, boundary keys (below min / above max), and heavy duplicates.
template <int Order>
static void range_query_check(const std::vector<int>& input) {
    std::multiset<int> ref(input.begin(), input.end());
    BTree<int, Order> tree(input.begin(), input.end());

    // Query a superset of keys: every value present, plus gaps and out-of-range.
    std::vector<int> queries;
    for (int v : input) { queries.push_back(v); queries.push_back(v - 1); queries.push_back(v + 1); }
    queries.push_back(std::numeric_limits<int>::min());
    queries.push_back(std::numeric_limits<int>::max());

    for (int q : queries) {
        auto rlo = ref.lower_bound(q);
        auto tlo = tree.lower_bound(q);
        ASSERT_EQ(tlo == tree.end(), rlo == ref.end());
        if (rlo != ref.end()) ASSERT_EQ(*tlo, *rlo);

        auto rhi = ref.upper_bound(q);
        auto thi = tree.upper_bound(q);
        ASSERT_EQ(thi == tree.end(), rhi == ref.end());
        if (rhi != ref.end()) ASSERT_EQ(*thi, *rhi);

        // count matches, and equal_range spans exactly that many, all equal to q.
        size_t expected = ref.count(q);
        ASSERT_EQ(tree.count(q), expected);

        auto er = tree.equal_range(q);
        size_t span = 0;
        for (auto it = er.first; it != er.second; ++it) {
            ASSERT_EQ(*it, q);
            ++span;
        }
        ASSERT_EQ(span, expected);
        ASSERT_TRUE(er.first == tlo);   // equal_range.first == lower_bound
        ASSERT_TRUE(er.second == thi);  // equal_range.second == upper_bound
    }
}

TEST(test_range_queries_differential) {
    std::mt19937 rng(31337);
    for (size_t n : {size_t(0), size_t(1), size_t(2), size_t(5), size_t(50),
                     size_t(500), size_t(3000)}) {
        std::vector<int> data(n);
        for (size_t i = 0; i < n; ++i) data[i] = static_cast<int>(rng() % 200);  // dup-heavy
        range_query_check<3>(data);
        range_query_check<5>(data);
        range_query_check<64>(data);
        range_query_check<100>(data);
    }
}

// A key type that tallies every comparison it participates in, to prove that
// count()/equal_range descend the tree (O(log n + k) comparisons) instead of
// scanning every element (O(n)).
struct CmpCounter {
    int v;
    static long cmps;
    CmpCounter(int x = 0) : v(x) {}
    bool operator<(const CmpCounter& o) const { ++cmps; return v < o.v; }
    bool operator==(const CmpCounter& o) const { ++cmps; return v == o.v; }
};
long CmpCounter::cmps = 0;

// count() must use a logarithmic descent, not a linear scan: for a large tree
// it performs dramatically fewer key comparisons than std::count over the whole
// range. Pre-change there is no count(), so this fails to compile.
TEST(test_count_is_sublinear) {
    const int N = 20000;
    std::vector<CmpCounter> data;
    data.reserve(N);
    std::mt19937 rng(7);
    for (int i = 0; i < N; ++i) data.emplace_back(static_cast<int>(rng() % 1000));
    BTree<CmpCounter, 64> tree(data.begin(), data.end());

    const CmpCounter probe(500);

    CmpCounter::cmps = 0;
    size_t c_tree = tree.count(probe);
    long cmps_tree = CmpCounter::cmps;

    CmpCounter::cmps = 0;
    long c_scan = std::count(tree.begin(), tree.end(), probe);
    long cmps_scan = CmpCounter::cmps;

    ASSERT_EQ(c_tree, static_cast<size_t>(c_scan));  // same answer
    ASSERT_TRUE(cmps_scan >= N);                     // full scan touches every element
    // The descent must be at least ~20x cheaper in comparisons than the scan.
    ASSERT_TRUE(cmps_tree * 20 < cmps_scan);
}

// count() must reach its answer in a SINGLE root-to-leaf descent, not two: its
// per-query comparison work should be within ~1.4x of a single find(). A
// two-descent implementation (one pass for lower_bound, one for upper_bound)
// runs at ~1.7-1.9x find(), so this fails before the single-descent change and
// passes after. Aggregated over many random queries for a stable ratio.
TEST(test_count_single_descent) {
    const int N = 50000;
    std::vector<CmpCounter> data;
    data.reserve(N);
    std::mt19937 rng(7);
    for (int i = 0; i < N; ++i) data.emplace_back(static_cast<int>(rng() % 100000));
    BTree<CmpCounter, 16> tree(data.begin(), data.end());

    long find_total = 0, count_total = 0;
    std::mt19937 q(999);
    for (int t = 0; t < 200; ++t) {
        CmpCounter probe(static_cast<int>(q() % 100000));
        CmpCounter::cmps = 0;
        (void)tree.find(probe);
        find_total += CmpCounter::cmps;
        CmpCounter::cmps = 0;
        (void)tree.count(probe);
        count_total += CmpCounter::cmps;
    }
    ASSERT_TRUE(find_total > 0 && count_total > 0);
    ASSERT_TRUE(count_total * 10 <= find_total * 14);  // count comparisons <= 1.4x find
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
    RUN_TEST(test_size);
    RUN_TEST(test_remove_basic);
    RUN_TEST(test_remove_nonexistent);
    RUN_TEST(test_remove_empty);
    RUN_TEST(test_remove_all);
    RUN_TEST(test_remove_rebalancing);
    RUN_TEST(test_remove_reverse);
    RUN_TEST(test_remove_strong_exception_safety);
    RUN_TEST(test_remove_rebalance_throw_strong);
    RUN_TEST(test_insert_strong_exception_safety_alloc_fail);
    RUN_TEST(test_move_constructor);
    RUN_TEST(test_move_assignment);
    RUN_TEST(test_stress_insert_remove);

    // Edge case tests
    RUN_TEST(test_remove_single_element);
    RUN_TEST(test_remove_minimum);
    RUN_TEST(test_remove_maximum);
    RUN_TEST(test_reinsert_after_remove);
    RUN_TEST(test_alternating_insert_remove);
    RUN_TEST(test_remove_middle_elements);
    RUN_TEST(test_remove_twice);
    RUN_TEST(test_order_3_edge_cases);
    RUN_TEST(test_order_50);
    RUN_TEST(test_cascade_merge);
    RUN_TEST(test_insert_after_empty);
    RUN_TEST(test_string_edge_cases);
    RUN_TEST(test_size_consistency);
    RUN_TEST(test_interleaved_stress);
    RUN_TEST(test_random_operations_integrity);

    // New API tests
    RUN_TEST(test_contains);
    RUN_TEST(test_clear);
    RUN_TEST(test_height);
    RUN_TEST(test_min);
    RUN_TEST(test_max);
    RUN_TEST(test_for_each);
    RUN_TEST(test_to_vector);
    RUN_TEST(test_iterator_basic);
    RUN_TEST(test_range_based_for);
    RUN_TEST(test_iterator_large);
    RUN_TEST(test_iterator_empty);
    RUN_TEST(test_const_iterators);
    RUN_TEST(test_traverse_ostream);
    RUN_TEST(test_iterator_post_increment);
    RUN_TEST(test_stl_algorithms);
    RUN_TEST(test_find_method);

    // Additional tests - Empty tree edge cases
    RUN_TEST(test_to_vector_empty);
    RUN_TEST(test_for_each_empty);
    RUN_TEST(test_traverse_empty);

    // Additional tests - Iterator edge cases
    RUN_TEST(test_iterator_arrow_operator);
    RUN_TEST(test_iterator_single_element);
    RUN_TEST(test_iterator_equality);
    RUN_TEST(test_iterator_string_arrow);

    // Additional tests - Duplicate handling
    RUN_TEST(test_duplicate_size_behavior);
    RUN_TEST(test_multiple_duplicates);
    RUN_TEST(test_remove_one_duplicate);

    // Additional tests - Move semantics edge cases
    RUN_TEST(test_self_move_assignment);
    RUN_TEST(test_move_empty_tree);

    // Additional tests - Height verification
    RUN_TEST(test_height_growth);
    RUN_TEST(test_height_after_removals);

    // Additional tests - Min/Max edge cases
    RUN_TEST(test_min_max_through_modifications);
    RUN_TEST(test_min_max_negative_positive);

    // Additional tests - Clear edge cases
    RUN_TEST(test_clear_empty_tree);
    RUN_TEST(test_multiple_clears);

    // Additional tests - Find edge cases
    RUN_TEST(test_find_with_duplicates);
    RUN_TEST(test_find_returns_first_duplicate);
    RUN_TEST(test_find_after_modifications);

    // Additional tests - Higher-order remove stress
    RUN_TEST(test_order_4_random_remove_stress);
    RUN_TEST(test_order_6_remove_sequential);
    RUN_TEST(test_order_7_interleaved_stress);

    // Additional tests - Large scale stress
    RUN_TEST(test_large_scale_10k);
    RUN_TEST(test_large_scale_50k_order_10);

    // Additional tests - String stress
    RUN_TEST(test_long_strings);
    RUN_TEST(test_special_string_patterns);

    // Additional tests - Custom type
    RUN_TEST(test_custom_comparable_type);

    // Additional tests - Traverse order verification
    RUN_TEST(test_traverse_strict_order);

    // Edge condition tests - Tier 1: Critical
    RUN_TEST(test_root_collapse_internal);
    RUN_TEST(test_remove_case_2c_recursive);
    RUN_TEST(test_order_4_merge_split_cycle);
    RUN_TEST(test_borrow_from_right_sibling);
    RUN_TEST(test_rightmost_child_merge_left);
    RUN_TEST(test_iterator_after_remove);
    RUN_TEST(test_order_4_comprehensive);
    RUN_TEST(test_root_split_full_leaf);

    // Edge condition tests - Tier 2: High Priority
    RUN_TEST(test_fill_child_exact_min_keys);
    RUN_TEST(test_borrow_left_boundary);
    RUN_TEST(test_borrow_right_boundary);
    RUN_TEST(test_merge_parent_no_overflow);
    RUN_TEST(test_height_reduction_exact_moment);
    RUN_TEST(test_insert_cascade_splits);
    RUN_TEST(test_merge_split_mid_calculation);
    RUN_TEST(test_fill_child_is_last_flag);

    // Edge condition tests - Tier 3: Medium Priority
    RUN_TEST(test_find_duplicate_iterator_validity);
    RUN_TEST(test_iterator_cross_tree_comparison);
    RUN_TEST(test_get_predecessor_shapes);
    RUN_TEST(test_get_successor_shapes);
    RUN_TEST(test_height_stable_after_removes);
    RUN_TEST(test_order_5_min_keys_boundary);

    // Duplicate-heavy coverage: differential vs std::multiset, leftmost-find
    // half-open range, moved-from reuse, and all-identical-key churn.
    RUN_TEST(test_multiset_differential_stress);
    RUN_TEST(test_find_halfopen_range_duplicates);
    RUN_TEST(test_reuse_populated_tree_after_move);
    RUN_TEST(test_all_identical_insert_remove);

    // Move-aware insert(T&&): rvalue keys are relocated, not copied.
    RUN_TEST(test_insert_rvalue_moves_not_copies);
    RUN_TEST(test_insert_lvalue_preserves_source);

    // Bulk-load range/initializer_list constructor.
    RUN_TEST(test_bulk_load_range_constructor);
    RUN_TEST(test_bulk_load_initializer_list);
    RUN_TEST(test_bulk_load_find_leftmost_duplicate);

    // Range queries: lower_bound / upper_bound / equal_range / count.
    RUN_TEST(test_range_queries_differential);
    RUN_TEST(test_count_is_sublinear);
    RUN_TEST(test_count_single_descent);

    std::cout << std::endl;
    std::cout << "=== Results ===" << std::endl;
    std::cout << "Passed: " << tests_passed << std::endl;
    std::cout << "Failed: " << tests_failed << std::endl;

    return tests_failed == 0 ? 0 : 1;
}
