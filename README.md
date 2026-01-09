# BTree

A C++ implementation of a B-tree data structure.

## Overview

This project provides a generic, templated B-tree implementation supporting:
- Configurable order (default: 3)
- Any comparable key type (int, string, double, etc.)
- Insert, search, and remove operations
- In-order traversal
- Move semantics

## Building

Compile the main implementation:

```bash
g++ -std=c++17 -Wall -Wextra -o btree btree.cpp
```

Compile with debug symbols:

```bash
g++ -std=c++17 -Wall -Wextra -g -o btree btree.cpp
```

## Usage

```cpp
#include "btree.cpp"

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

// Check if empty
if (!tree.empty()) {
    std::cout << "Tree has elements" << std::endl;
}

// Print all values in order
tree.traverse();

// Create a BTree with custom order
BTree<std::string, 5> string_tree;
string_tree.insert("apple");
string_tree.insert("banana");
```

## Running Tests

Compile and run the test suite:

```bash
g++ -std=c++17 -Wall -Wextra -o btree_test btree_test.cpp && ./btree_test
```

The test suite includes 19 tests covering:
- Basic operations (insert, search, empty)
- Sorted and reverse-sorted insertions
- Random order insertions
- Different data types (int, string, double)
- Different tree orders (3, 5, 10)
- Edge cases (duplicates, negative values, INT_MIN/INT_MAX)
- Stress testing with 1000 elements

## API Reference

### `BTree<T, Order>`

Template parameters:
- `T` - Key type (must support comparison operators)
- `Order` - B-tree order (default: 3)

Methods:
- `void insert(const T& key)` - Insert a key into the tree
- `bool remove(const T& key)` - Remove a key, returns true if found
- `bool search(const T& key) const` - Returns true if key exists
- `void traverse() const` - Print all keys in sorted order
- `bool empty() const` - Returns true if tree is empty
- `size_t size() const` - Returns number of keys in the tree

Note: Copy operations are disabled. Use `std::move()` to transfer ownership.
