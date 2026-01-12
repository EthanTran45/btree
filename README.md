# BTree

A header-only C++ implementation of a B-tree data structure.

## Overview

This project provides a generic, templated B-tree implementation supporting:
- Configurable order (default: 3, recommended: 4+)
- Any comparable key type (int, string, double, etc.)
- Insert, search, remove, and find operations
- In-order traversal with STL-compatible iterators
- Binary search within nodes for O(log k) performance
- Move semantics

**Note:** Order 3 has a known issue with `remove()` for certain random deletion patterns. For production use, Order >= 4 is recommended.

## Building

This is a header-only library. Simply include `btree.hpp` in your project:

```cpp
#include "btree.hpp"
```

To compile a program using the library:

```bash
g++ -std=c++17 -Wall -Wextra -o myprogram myprogram.cpp
```

## Usage

```cpp
#include "btree.hpp"

// Create a BTree with default order (3)
BTree<int> tree;

// Insert values
tree.insert(10);
tree.insert(20);
tree.insert(5);

// Search for values
if (tree.search(10)) {
    std::cout << "Found 10" << std::endl;
}

// Find returns an iterator
auto it = tree.find(10);
if (it != tree.end()) {
    std::cout << "Found: " << *it << std::endl;
}

// Check if empty
if (!tree.empty()) {
    std::cout << "Size: " << tree.size() << std::endl;
}

// Remove values
if (tree.remove(20)) {
    std::cout << "Removed 20" << std::endl;
}

// Print all values in order
tree.traverse();

// Range-based for loop
for (const auto& key : tree) {
    std::cout << key << " ";
}

// Create a BTree with custom order (recommended: 4+)
BTree<std::string, 5> string_tree;
string_tree.insert("apple");
string_tree.insert("banana");

// Move semantics (copy is disabled)
BTree<int> tree2 = std::move(tree);  // tree is now empty
```

## Running Tests

Compile and run the test suite:

```bash
g++ -std=c++17 -Wall -Wextra -o btree_test btree_test.cpp && ./btree_test
```

The test suite includes 111 tests organized into the following categories:

### Basic Operations (19 tests)
- Empty tree behavior and state transitions
- Single and multiple insertions
- Sorted, reverse-sorted, and random order insertions
- Search for existing and non-existing keys
- Different data types (int, string, double)
- Different tree orders (3, 5, 10, 50)
- Boundary values (INT_MIN, INT_MAX)
- Traverse output verification

### Remove Operations (10 tests)
- Basic removal and size updates
- Removing non-existent keys (returns false)
- Removing from empty tree
- Removing all elements until empty
- Rebalancing with borrow from siblings
- Cascade merging when nodes underflow
- Removing in reverse order

### Move Semantics (2 tests)
- Move constructor transfers ownership
- Move assignment cleans up existing data

### Edge Cases (13 tests)
- Remove single-element tree
- Remove minimum/maximum elements repeatedly
- Re-insert after removal
- Alternating insert and remove operations
- Remove middle elements (internal node operations)
- Remove same element twice
- Minimum order (Order=3) stress test
- Large order (Order=50) with 1000 elements
- Insert after tree has been emptied
- Empty string and similar prefix handling
- Size consistency after failed operations
- Interleaved stress test (500 inserts, 250 removes)
- Random operations validated against std::set

### New API (16 tests)
- contains() method
- clear() method
- height() method
- min()/max() methods with exception handling
- for_each() with lambda functions
- to_vector() conversion
- Iterator basic usage and post-increment
- Range-based for loops
- Iterator on empty and large trees
- Const iterators (cbegin/cend)
- traverse() with custom ostream
- STL algorithm compatibility (std::find, std::count)
- find() method returning iterator

### Additional Tests (51 tests)
- Move semantics edge cases (self-move, empty tree move)
- Height verification and growth patterns
- Min/max through modifications
- Clear edge cases
- Find with duplicates and after modifications
- Higher-order remove stress tests (Order 4, 6, 7)
- Large scale tests (10K, 50K operations)
- String stress tests and custom comparable types
- Critical B-tree edge conditions (root collapse, case 2c recursive, merge/split cycles)
- Iterator validity and cross-tree comparison

## Running Benchmarks

Compile and run the benchmark suite:

```bash
g++ -std=c++17 -O2 -o btree_benchmark btree_benchmark.cpp && ./btree_benchmark
```

The benchmark compares BTree performance against `std::set` across different tree orders (3, 10, 50, 100) and data sizes (10K, 100K, 1M elements). Operations tested include insert, search, find, iteration, and remove (Order >= 4 only).

You can specify custom sizes via command line:

```bash
./btree_benchmark 50000 200000
```

## API Reference

### `BTree<T, Order>`

Template parameters:
- `T` - Key type (must support comparison operators)
- `Order` - B-tree order (default: 3)

#### Core Methods
| Method | Complexity | Description |
|--------|------------|-------------|
| `void insert(const T& key)` | O(log n) | Insert a key into the tree |
| `bool remove(const T& key)` | O(log n) | Remove a key, returns true if found |
| `bool search(const T& key) const` | O(log n) | Returns true if key exists |
| `bool contains(const T& key) const` | O(log n) | Alias for search (STL-style) |
| `iterator find(const T& key) const` | O(log n) | Returns iterator to key, or end() if not found |

#### Traversal
| Method | Complexity | Description |
|--------|------------|-------------|
| `void traverse() const` | O(n) | Print all keys in sorted order to stdout |
| `void traverse(std::ostream& os) const` | O(n) | Print all keys to custom stream |
| `void for_each(Func f) const` | O(n) | Apply function to each key in order |
| `std::vector<T> to_vector() const` | O(n) | Return all keys as sorted vector |

#### Utility
| Method | Complexity | Description |
|--------|------------|-------------|
| `bool empty() const` | O(1) | Returns true if tree is empty |
| `size_t size() const` | O(1) | Returns number of keys |
| `size_t height() const` | O(log n) | Returns height of tree (0 if empty) |
| `void clear()` | O(n) | Remove all keys |
| `const T& min() const` | O(log n) | Returns smallest key (throws if empty) |
| `const T& max() const` | O(log n) | Returns largest key (throws if empty) |

#### Iterators
| Method | Complexity | Description |
|--------|------------|-------------|
| `iterator begin() const` | O(log n) | Iterator to first element |
| `iterator end() const` | O(1) | Iterator past last element |
| `const_iterator cbegin() const` | O(log n) | Const iterator to first element |
| `const_iterator cend() const` | O(1) | Const iterator past last element |

Iterator increment (`++it`) is amortized O(1).

Iterators support range-based for loops and STL algorithms:
```cpp
// Range-based for loop
for (const auto& key : tree) {
    std::cout << key << std::endl;
}

// Built-in find (O(log n) using binary search)
auto it = tree.find(42);

// STL algorithms also work
auto it2 = std::find(tree.begin(), tree.end(), 42);
int count = std::count(tree.begin(), tree.end(), 42);
```

Note: Copy operations are disabled. Use `std::move()` to transfer ownership.

## Iterator Invalidation

**Warning:** Unlike `std::map`/`std::set`, ALL iterators are invalidated when the tree is modified:

- `insert()` - invalidates all iterators (may cause node splits)
- `remove()` - invalidates all iterators (may cause node merges)
- `clear()` - invalidates all iterators

Only use iterators while the tree structure is unchanged.
