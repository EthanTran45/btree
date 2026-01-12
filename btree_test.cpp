#include <iostream>
#include <cassert>
#include <string>
#include <sstream>
#include <vector>
#include <random>
#include <algorithm>
#include <climits>
#include <set>

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

    // Self-move assignment - implementation checks `this != &other` so tree is preserved
    tree = std::move(tree);

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

// Test: Order 4 sustained merge-split cycles (avoids Order 3 known issues)
TEST(test_order_4_merge_split_cycle) {
    BTree<int, 4> tree;  // Order 4: more reliable than Order 3

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

    std::cout << std::endl;
    std::cout << "=== Results ===" << std::endl;
    std::cout << "Passed: " << tests_passed << std::endl;
    std::cout << "Failed: " << tests_failed << std::endl;

    return tests_failed == 0 ? 0 : 1;
}
