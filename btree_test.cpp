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

    std::cout << std::endl;
    std::cout << "=== Results ===" << std::endl;
    std::cout << "Passed: " << tests_passed << std::endl;
    std::cout << "Failed: " << tests_failed << std::endl;

    return tests_failed == 0 ? 0 : 1;
}
