# BTree

A header-only C++ implementation of a B-tree data structure.

## Overview

This project provides a generic, templated B-tree implementation supporting:
- Configurable order (default: 64; any order >= 3)
- Any comparable key type (int, string, double, etc.)
- Insert, search, remove, and find operations
- Duplicate keys (multiset semantics; `find` returns the first/leftmost match)
- In-order traversal with STL-compatible iterators
- Cache-friendly node layout: each node is a single allocation with its keys
  stored inline (see [Implementation notes](#implementation-notes))
- Move semantics, including a move-aware `insert(T&&)` that relocates the key
  into its leaf instead of copying it
- Strong exception-safety guarantee for `insert` and `remove` (see
  [Exception safety](#exception-safety))

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

// Create a BTree with default order (64)
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

// Create a BTree with custom order
BTree<std::string, 5> string_tree;
string_tree.insert("apple");
string_tree.insert("banana");

// Move a key straight into the tree (no copy of the string payload)
std::string big = build_large_key();
string_tree.insert(std::move(big));  // big is left in a valid moved-from state

// Move semantics (copy is disabled)
BTree<int> tree2 = std::move(tree);  // tree is now empty
```

## Running Tests

Compile and run the test suite:

```bash
g++ -std=c++17 -Wall -Wextra -o btree_test btree_test.cpp && ./btree_test
```

The test suite includes 121 tests organized into the following categories:

### Basic Operations (19 tests)
- Empty tree behavior and state transitions
- Single and multiple insertions
- Sorted, reverse-sorted, and random order insertions
- Search for existing and non-existing keys
- Different data types (int, string, double)
- Different tree orders (3, 5, 10, 50)
- Boundary values (INT_MIN, INT_MAX)
- Traverse output verification

### Remove Operations (13 tests)
- Basic removal and size updates
- Removing non-existent keys (returns false)
- Removing from empty tree
- Removing all elements until empty
- Rebalancing with borrow from siblings
- Cascade merging when nodes underflow
- Removing in reverse order
- Strong exception guarantee: a throwing key-copy during `remove()` leaves the
  tree completely unchanged
- Strong exception guarantee when the throw lands mid-rebalance (borrow/merge)
- Strong exception guarantee for `insert()` when a node allocation fails during
  a split cascade

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

### Additional Tests (58 tests)
- Move semantics edge cases (self-move, empty tree move, reuse after move)
- Height verification and growth patterns
- Min/max through modifications
- Clear edge cases
- Find with duplicates and after modifications; `find` returns the leftmost match
- Higher-order remove stress tests (Order 4, 6, 7)
- Large scale tests (10K, 50K operations)
- String stress tests and custom comparable types
- Critical B-tree edge conditions (root collapse, case 2c recursive, merge/split cycles)
- Iterator validity and cross-tree comparison
- Duplicate-heavy coverage: randomized differential stress against `std::multiset`,
  leftmost-`find` half-open range iteration, and all-identical-key insert/remove churn
- Move-aware `insert(T&&)`: an rvalue key is relocated into its leaf with zero
  copies (proven with a copy/move-counting key type), while the lvalue overload
  still copies and leaves the caller's object intact

## Running Benchmarks

Compile and run the benchmark suite:

```bash
g++ -std=c++17 -O2 -o btree_benchmark btree_benchmark.cpp && ./btree_benchmark
```

The benchmark compares BTree performance against `std::set` across different tree orders (3, 10, 50, 64, 100) and data sizes (10K, 100K, 1M elements). Operations tested include insert, search, find, iteration, and remove.

You can specify custom sizes via command line:

```bash
./btree_benchmark 50000 200000
```

### Sample Results

Measured at **1,000,000 elements**, compiled with `g++ -O2` (GCC 16, Windows). Values are wall-clock **milliseconds for the whole batch of 1M operations — lower is better** (best of 3 runs). `std::set` is the red-black-tree baseline; its workload is random-order. Absolute numbers are machine-dependent, so treat the relative comparison as the takeaway.

| Operation            | Order 3 | Order 10 | Order 50 | Order 64 (default) | Order 100 | `std::set` |
|----------------------|--------:|---------:|---------:|-------------------:|----------:|-----------:|
| insert (random)      |   408   |    113   |     90   |         99         |    114    |    503     |
| insert (sequential)  |    74   |     19   |    9.4   |        8.7         |    8.2    |     —      |
| search               |   452   |    131   |     90   |         88         |    109    |    630     |
| find (iterator)      |   491   |    145   |    101   |        101         |    117    |    615     |
| iterate (full scan)  |    23   |    4.6   |    2.3   |        2.2         |    2.1    |    109     |
| remove (random)      |   548   |    172   |    102   |        116         |    124    |    745     |

Takeaways:
- **Default order 64 vs `std::set` at 1M:** ~5× faster to build, ~7× faster to search, ~6× faster to find/remove, and **~51× faster** to iterate in sorted order — inline, contiguous keys make both the descent and the full scan far more cache-friendly than pointer-chasing a red-black tree.
- **Higher orders are faster, up to a sweet spot around 50–100.** Larger nodes mean a shallower tree and fewer cache misses; a branchless in-node key scan keeps per-node work cheap even at order 100. Order 3 is the slowest (deepest tree, most per-node overhead) but is correct and still comfortably beats `std::set`.
- **Sequential insertion is nearly free** thanks to an O(1) append fast-path (a sorted-order bulk load of 1M ints takes <10 ms at the larger orders).
- **Versus the previous `std::vector`-per-node implementation** (same machine, same benchmark): roughly **3× faster** across the board at order 3 and **1.3–1.7×** at order 100, with the largest gains from the single-allocation inline node layout and the branchless in-node search.

## Implementation notes

The library keeps the same B-tree algorithm and public API as before, but the
data layout and inner loops are tuned for cache and branch behavior:

- **Single-allocation inline nodes.** Each node stores its keys (and, for
  internal nodes, its child pointers) in fixed-capacity buffers embedded in the
  node itself (`btree_detail::InlineVec`) instead of two separate `std::vector`
  members. A node is therefore one allocation with its keys co-resident with its
  header, roughly halving cache misses per level. Element moves are gated on
  `std::is_trivially_copyable`: trivial types (`int`, `double`, aggregates) move
  with `memmove`/`memcpy`; non-trivial types (`std::string`) move element-wise so
  internal self-pointers are never byte-copied.
- **Leaf-aware sizing.** Leaves are a distinct, smaller node type that omits the
  child-pointer array, so the ~99% of nodes that are leaves carry no wasted
  storage — important for keeping large trees resident in cache at high orders.
- **Per-tree slab allocator.** Nodes are carved from large slabs (one `malloc`
  per slab, not per node) with a free-list for reuse, cutting allocator traffic
  and improving locality. Teardown is O(#slabs) for trivially-destructible keys.
- **Branchless in-node search.** For arithmetic keys the in-node position search
  is a mispredict-free counting scan (which the compiler vectorizes) instead of a
  branchy binary search, with O(1) append/prepend fast-paths so sorted insertion
  stays cheap. Other key types keep `std::lower_bound`/`upper_bound`.
- **Allocation-free iterators.** The iterator's traversal stack lives inline
  (spilling to the heap only for pathologically deep trees), so `find()` and
  `begin()` do no heap allocation.

The default order is **64**, a cache-line-friendly fan-out in the measured sweet
spot; any order `>= 3` remains supported and correct.

## API Reference

### `BTree<T, Order>`

Template parameters:
- `T` - Key type (must support comparison operators)
- `Order` - B-tree order (default: 64)

#### Core Methods
| Method | Complexity | Description |
|--------|------------|-------------|
| `void insert(const T& key)` | O(log n) | Insert a key into the tree (copies the key) |
| `void insert(T&& key)` | O(log n) | Insert a key, moving from `key` (one fewer key copy for movable types) |
| `bool remove(const T& key)` | O(log n) | Remove a key, returns true if found |
| `bool search(const T& key) const` | O(log n) | Returns true if key exists |
| `bool contains(const T& key) const` | O(log n) | Alias for search (STL-style) |
| `iterator find(const T& key) const` | O(log n) | Returns iterator to the first (leftmost) match, or end() if not found |

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
| `size_t height() const` | O(1) | Returns height of tree (0 if empty) |
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

## Exception safety

`insert()` and `remove()` provide the **strong exception guarantee**: if the
operation throws — whether from a throwing key copy/comparison or from an
allocation failure while growing the tree — the tree is left exactly as it was
before the call. No element is lost or duplicated, `size()` and `empty()` stay
consistent, and nothing is leaked. Both operations copy the incoming (or
predecessor) value *before* mutating any node, then perform all buffer shuffling
by move, so once the mutation begins it cannot fail partway through. The
move-aware `insert(T&&)` overload instead *moves* the key into its leaf, but a
move is `noexcept` for any supported `T`, so its commit point cannot throw
either; only the (unchanged) key comparisons during the descent can, and those
run before any node is touched.

This relies on the element type being cheap and infallible to relocate, which is
enforced at compile time:

```cpp
static_assert(std::is_nothrow_move_constructible_v<T> &&
              std::is_nothrow_move_assignable_v<T>);
```

`T` must be nothrow-move-constructible and nothrow-move-assignable (the standard
library types you would reach for — `int`, `double`, `std::string`, etc. — all
qualify). `T`'s *copy* constructor and comparison operators may throw; those
throwing paths are exactly what the guarantee protects against.

`search()` / `contains()` are `noexcept` when `T`'s comparison is `noexcept`
(always so for arithmetic keys and `std::string`). `min()` and `max()` throw
`std::runtime_error` when called on an empty tree.

## Iterator Invalidation

**Warning:** Unlike `std::map`/`std::set`, ALL iterators are invalidated when the tree is modified:

- `insert()` - invalidates all iterators (may cause node splits)
- `remove()` - invalidates all iterators (may cause node merges)
- `clear()` - invalidates all iterators

Only use iterators while the tree structure is unchanged.
