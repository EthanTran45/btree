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
    std::cout << "Size: " << tree.size() << std::endl;
}

// Remove values
if (tree.remove(20)) {
    std::cout << "Removed 20" << std::endl;
}

// Print all values in order
tree.traverse();

// Create a BTree with custom order
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

The test suite includes 59 tests organized into the following categories:

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

### New API (15 tests)
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

## API Reference

### `BTree<T, Order>`

Template parameters:
- `T` - Key type (must support comparison operators)
- `Order` - B-tree order (default: 3)

#### Core Methods
| Method | Description |
|--------|-------------|
| `void insert(const T& key)` | Insert a key into the tree |
| `bool remove(const T& key)` | Remove a key, returns true if found |
| `bool search(const T& key) const` | Returns true if key exists |
| `bool contains(const T& key) const` | Alias for search (STL-style) |

#### Traversal
| Method | Description |
|--------|-------------|
| `void traverse() const` | Print all keys in sorted order to stdout |
| `void traverse(std::ostream& os) const` | Print all keys to custom stream |
| `void for_each(Func f) const` | Apply function to each key in order |
| `std::vector<T> to_vector() const` | Return all keys as sorted vector |

#### Utility
| Method | Description |
|--------|-------------|
| `bool empty() const` | Returns true if tree is empty |
| `size_t size() const` | Returns number of keys |
| `size_t height() const` | Returns height of tree (0 if empty) |
| `void clear()` | Remove all keys |
| `const T& min() const` | Returns smallest key (throws if empty) |
| `const T& max() const` | Returns largest key (throws if empty) |

#### Iterators
| Method | Description |
|--------|-------------|
| `iterator begin() const` | Iterator to first element |
| `iterator end() const` | Iterator past last element |
| `const_iterator cbegin() const` | Const iterator to first element |
| `const_iterator cend() const` | Const iterator past last element |

Iterators support range-based for loops and STL algorithms:
```cpp
// Range-based for loop
for (const auto& key : tree) {
    std::cout << key << std::endl;
}

// STL algorithms
auto it = std::find(tree.begin(), tree.end(), 42);
```

Note: Copy operations are disabled. Use `std::move()` to transfer ownership.
