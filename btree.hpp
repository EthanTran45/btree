#pragma once

#include <iostream>
#include <vector>
#include <stdexcept>
#include <functional>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>
#include <iterator>
#include <initializer_list>

// B-tree implementation with configurable order (any Order >= 3).
//
// Uses reactive (bottom-up) rebalancing: inserts descend to a leaf and split
// overflowed nodes on the way back up; removes erase at a leaf and repair
// underflowed nodes on the way back up. This keeps every node within
// [min_keys, max_keys] and all leaves at the same depth for every order.
//
// Node storage is inline: each node holds its keys (and, for internal nodes,
// its child pointers) in fixed-capacity buffers embedded in the node itself
// (see InlineVec), so a node is a *single* heap allocation with its keys
// co-resident with its header. Leaf nodes are a distinct, smaller type that
// omits the child-pointer array entirely, so the ~99% of nodes that are leaves
// carry no wasted storage. Together this roughly halves cache misses per level
// versus a std::vector-per-node layout and removes two allocator round-trips
// per node.
//
// Iterator Invalidation:
// - insert(): Invalidates all iterators (may cause node splits/reallocations)
// - remove(): Invalidates all iterators (may cause node merges/reallocations)
// - clear(): Invalidates all iterators
// - Iterators are safe to use only while the tree structure is unchanged.
// - Unlike std::map/std::set, ALL iterators are invalidated on any mutation.

namespace btree_detail {

// Fixed-capacity, inline-storage container exposing the subset of the
// std::vector API the B-tree uses. Elements live in a buffer embedded in the
// object (no separate heap allocation). All element relocation is gated on
// std::is_trivially_copyable_v<U>: trivial types (int, double, aggregates like
// Point, and Node* pointers) move with memmove/memcpy; non-trivial types
// (std::string) move element-wise via their move/copy constructors so an
// object's internal self-pointers are never byte-copied.
template <typename U, int Cap>
class InlineVec {
    static_assert(Cap > 0, "InlineVec capacity must be positive");

    alignas(U) unsigned char storage_[sizeof(U) * static_cast<std::size_t>(Cap)];
    // 32-bit count: a node transiently holds up to Order keys / Order+1 children
    // before a split resolves the overflow, so a 16-bit counter would wrap (and
    // silently drop the whole node) once Order reaches 65536. uint32_t covers
    // every realistic order at a cost of two bytes per node.
    std::uint32_t size_ = 0;

    U* data() noexcept { return std::launder(reinterpret_cast<U*>(storage_)); }
    const U* data() const noexcept {
        return std::launder(reinterpret_cast<const U*>(storage_));
    }

    void destroy_all() noexcept {
        if constexpr (!std::is_trivially_destructible_v<U>) {
            U* d = data();
            for (std::uint32_t i = 0; i < size_; ++i) {
                d[i].~U();
            }
        }
    }

public:
    InlineVec() noexcept = default;
    ~InlineVec() { destroy_all(); }

    // Nodes are never copied or moved (the tree only moves the root pointer),
    // so neither is InlineVec — deleting them prevents accidental byte-copies.
    InlineVec(const InlineVec&) = delete;
    InlineVec& operator=(const InlineVec&) = delete;
    InlineVec(InlineVec&&) = delete;
    InlineVec& operator=(InlineVec&&) = delete;

    // Element access
    U* begin() noexcept { return data(); }
    U* end() noexcept { return data() + size_; }
    const U* begin() const noexcept { return data(); }
    const U* end() const noexcept { return data() + size_; }
    U* raw() noexcept { return data(); }

    std::size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }

    U& operator[](std::size_t i) noexcept { return data()[i]; }
    const U& operator[](std::size_t i) const noexcept { return data()[i]; }
    U& back() noexcept { return data()[size_ - 1]; }
    const U& back() const noexcept { return data()[size_ - 1]; }

    // Fixed capacity: reserve is a no-op, present only for API compatibility.
    void reserve(std::size_t) noexcept {}

    void clear() noexcept {
        destroy_all();
        size_ = 0;
    }

    void push_back(const U& value) {
        ::new (static_cast<void*>(data() + size_)) U(value);
        ++size_;
    }
    void push_back(U&& value) {
        ::new (static_cast<void*>(data() + size_)) U(std::move(value));
        ++size_;
    }

    void pop_back() noexcept {
        --size_;
        if constexpr (!std::is_trivially_destructible_v<U>) {
            data()[size_].~U();
        }
    }

    // Insert `value` before `pos`. Returns a pointer to the inserted element.
    U* insert(const U* pos, const U& value) {
        U* d = data();
        std::size_t i = static_cast<std::size_t>(pos - d);
        if constexpr (std::is_trivially_copyable_v<U>) {
            std::memmove(d + i + 1, d + i, (size_ - i) * sizeof(U));
            std::memcpy(static_cast<void*>(d + i), std::addressof(value), sizeof(U));
        } else {
            if (i == size_) {
                ::new (static_cast<void*>(d + size_)) U(value);
            } else {
                // Copy the incoming value into a temporary BEFORE touching the
                // buffer: if that copy throws, the container is left completely
                // unchanged (strong guarantee) and no half-constructed slot is
                // orphaned. Every step after this point is a move, which is
                // noexcept for the types the tree stores, so the gap-open cannot
                // throw partway through and leak or corrupt the node.
                U tmp(value);
                ::new (static_cast<void*>(d + size_)) U(std::move(d[size_ - 1]));
                for (std::size_t j = size_ - 1; j > i; --j) {
                    d[j] = std::move(d[j - 1]);
                }
                d[i] = std::move(tmp);
            }
        }
        ++size_;
        return d + i;
    }

    // Move-insert `value` before `pos`. Mirror of the copy overload, but the
    // incoming value is relocated by move rather than copied, so for a
    // noexcept-move U the whole operation is noexcept — no `tmp` staging is
    // needed since the caller has already agreed to move from `value`. The
    // B-tree's rebalancing (borrow_from_prev) uses this so a repair on the
    // delete unwind cannot throw and break remove()'s strong guarantee.
    U* insert(const U* pos, U&& value) {
        U* d = data();
        std::size_t i = static_cast<std::size_t>(pos - d);
        if constexpr (std::is_trivially_copyable_v<U>) {
            std::memmove(d + i + 1, d + i, (size_ - i) * sizeof(U));
            std::memcpy(static_cast<void*>(d + i), std::addressof(value), sizeof(U));
        } else {
            if (i == size_) {
                ::new (static_cast<void*>(d + size_)) U(std::move(value));
            } else {
                ::new (static_cast<void*>(d + size_)) U(std::move(d[size_ - 1]));
                for (std::size_t j = size_ - 1; j > i; --j) {
                    d[j] = std::move(d[j - 1]);
                }
                d[i] = std::move(value);
            }
        }
        ++size_;
        return d + i;
    }

    // Erase the element at `pos`. Returns a pointer to the following element.
    U* erase(const U* pos) noexcept {
        U* d = data();
        std::size_t i = static_cast<std::size_t>(pos - d);
        if constexpr (std::is_trivially_copyable_v<U>) {
            std::memmove(d + i, d + i + 1, (size_ - i - 1) * sizeof(U));
        } else {
            for (std::size_t j = i; j + 1 < size_; ++j) {
                d[j] = std::move(d[j + 1]);
            }
            d[size_ - 1].~U();
        }
        --size_;
        return d + i;
    }

    // Shrink (destroying the tail) or grow (default-constructing). The B-tree
    // only ever shrinks via resize (in split_child).
    void resize(std::size_t n) {
        U* d = data();
        if (n < size_) {
            if constexpr (!std::is_trivially_destructible_v<U>) {
                for (std::size_t i = n; i < size_; ++i) {
                    d[i].~U();
                }
            }
        } else {
            for (std::size_t i = size_; i < n; ++i) {
                ::new (static_cast<void*>(d + i)) U();
            }
        }
        size_ = static_cast<std::uint32_t>(n);
    }

    // Append `n` elements relocated from `src` (used by split/merge). Trivial
    // types are memcpy'd; non-trivial types are move-constructed.
    void append_move(U* src, std::size_t n) {
        U* d = data() + size_;
        if constexpr (std::is_trivially_copyable_v<U>) {
            std::memcpy(static_cast<void*>(d), static_cast<const void*>(src), n * sizeof(U));
        } else {
            for (std::size_t i = 0; i < n; ++i) {
                ::new (static_cast<void*>(d + i)) U(std::move(src[i]));
            }
        }
        size_ = static_cast<std::uint32_t>(size_ + n);
    }
};

// Slab allocator for fixed-size, fixed-alignment blocks with a free-list for
// O(1) reuse. Blocks are carved sequentially from large slabs (one malloc per
// slab instead of one per node), and freed blocks are recycled. This both cuts
// allocator traffic and improves locality (nodes born together sit together).
// One BTree owns one pool per node size class; it is moved with the tree and
// released wholesale on teardown.
class SlabPool {
    void* free_list_ = nullptr;         // singly-linked recycled blocks
    std::vector<void*> slabs_;          // raw slab allocations, freed on release()
    char* bump_ = nullptr;              // next unused byte in the current slab
    char* bump_end_ = nullptr;
    std::size_t block_size_ = 0;
    std::size_t block_align_ = 0;
    std::size_t blocks_per_slab_ = 0;
    std::size_t free_count_ = 0;        // blocks currently on free_list_ (for reserve())

    void add_slab() {
        std::size_t bytes = block_size_ * blocks_per_slab_;
        // Reserve the tracking slot first. If push_back had to grow slabs_ and
        // that growth threw *after* the slab allocation below, the slab would be
        // lost (never recorded, never freed). Reserving hoists the only throwing
        // step ahead of the raw ::operator new so the slab can't leak.
        slabs_.reserve(slabs_.size() + 1);
        void* slab = ::operator new(bytes, std::align_val_t(block_align_));
        slabs_.push_back(slab);
        bump_ = static_cast<char*>(slab);
        bump_end_ = bump_ + bytes;
    }

    void move_from(SlabPool& o) noexcept {
        free_list_ = o.free_list_;
        slabs_ = std::move(o.slabs_);
        bump_ = o.bump_;
        bump_end_ = o.bump_end_;
        block_size_ = o.block_size_;
        block_align_ = o.block_align_;
        blocks_per_slab_ = o.blocks_per_slab_;
        free_count_ = o.free_count_;
        o.free_list_ = nullptr;
        o.bump_ = nullptr;
        o.bump_end_ = nullptr;
        o.free_count_ = 0;
        // o keeps its size/align/blocks_per_slab config so it remains a usable
        // (empty) pool after being moved from.
    }

public:
    SlabPool() = default;
    SlabPool(const SlabPool&) = delete;
    SlabPool& operator=(const SlabPool&) = delete;
    SlabPool(SlabPool&& o) noexcept { move_from(o); }
    SlabPool& operator=(SlabPool&& o) noexcept {
        if (this != &o) {
            release();
            move_from(o);
        }
        return *this;
    }
    ~SlabPool() { release(); }

    // block_size must be >= sizeof(void*) (a free block stores the next pointer)
    // and a multiple of block_align (so every carved block stays aligned).
    void configure(std::size_t block_size, std::size_t block_align,
                   std::size_t blocks_per_slab) noexcept {
        block_size_ = block_size;
        block_align_ = block_align;
        blocks_per_slab_ = blocks_per_slab;
    }

    void* allocate() {
        if (free_list_ != nullptr) {
            void* p = free_list_;
            free_list_ = *static_cast<void**>(p);
            --free_count_;
            return p;
        }
        // Check remaining capacity without forming an invalid pointer. When the
        // pool has no current slab (fresh, released, or moved-from) bump_ is null,
        // and `nullptr + block_size_` would be undefined behavior; the null test
        // short-circuits that. Otherwise bump_ and bump_end_ point into the same
        // slab, so the subtraction is well-defined and also avoids the past-the-end
        // pointer that `bump_ + block_size_` forms when a slab is exactly full.
        if (bump_ == nullptr ||
            static_cast<std::size_t>(bump_end_ - bump_) < block_size_) {
            add_slab();
        }
        void* p = bump_;
        bump_ += block_size_;
        return p;
    }

    void deallocate(void* p) noexcept {
        *static_cast<void**>(p) = free_list_;
        free_list_ = p;
        ++free_count_;
    }

    // Ensure the next `n` allocate() calls are satisfied from the free-list, so
    // each is guaranteed not to reach ::operator new (hence cannot throw). Any
    // block growth this requires happens here, up front: callers use it to hoist
    // a would-be-mid-mutation allocation to a pre-commit point where a throw is
    // still harmless. May throw bad_alloc, but only from within this call.
    void reserve(std::size_t n) {
        while (free_count_ < n) {
            // Carve one block (from the current slab, or a fresh one) and park it
            // on the free-list. Mirrors allocate()'s bump-region capacity check,
            // including the null-slab short-circuit that avoids forming an invalid
            // pointer. Works across slab boundaries, so n may exceed a slab's
            // block count (e.g. a tall order-3 tree needing height reservations).
            if (bump_ == nullptr ||
                static_cast<std::size_t>(bump_end_ - bump_) < block_size_) {
                add_slab();
            }
            void* p = bump_;
            bump_ += block_size_;
            deallocate(p);  // pushes onto free_list_ and bumps free_count_
        }
    }

    // Free every slab at once (retains the size/align config so the pool can be
    // reused afterwards, e.g. after clear()).
    void release() noexcept {
        for (void* s : slabs_) {
            ::operator delete(s, std::align_val_t(block_align_));
        }
        slabs_.clear();
        free_list_ = nullptr;
        bump_ = nullptr;
        bump_end_ = nullptr;
        free_count_ = 0;
    }
};

}  // namespace btree_detail

// Default order 64: a cache-line-friendly fan-out that keeps the tree shallow
// and the per-node branchless key scan short. Any Order >= 3 is supported and
// correct; larger orders trade a little more per-node work for fewer levels.
template <typename T, int Order = 64>
class BTree {
private:
    // Keys are relocated by move throughout (node splits/merges, borrow-based
    // rebalancing, and InlineVec's noexcept erase/append_move). remove()'s strong
    // exception guarantee and InlineVec's noexcept relocation both depend on those
    // moves not throwing, so require it explicitly: a throwing-move T would
    // otherwise corrupt the tree or call std::terminate silently.
    static_assert(std::is_nothrow_move_constructible_v<T> &&
                      std::is_nothrow_move_assignable_v<T>,
                  "BTree<T> requires T to be nothrow move-constructible and "
                  "nothrow move-assignable");

    // A leaf node: just its keys. Capacity Order matches the reactive design's
    // transient overflow (a node may briefly hold Order keys before split_child
    // runs on the recursion unwind). This bound is tight for every order incl. 3.
    struct Node {
        btree_detail::InlineVec<T, Order> keys;
        bool is_leaf;
        explicit Node(bool leaf = true) : is_leaf(leaf) {}
    };

    // An internal node additionally owns child pointers. children.size() is
    // always keys.size()+1; capacity Order+1 covers the transient overflow.
    struct InternalNode : Node {
        btree_detail::InlineVec<Node*, Order + 1> children;
        InternalNode() : Node(false) {}
    };

    // Accessor for an internal node's children. Only valid when !n->is_leaf;
    // every call site is guarded by an is_leaf check. Asserting that
    // precondition to the optimizer both improves codegen and silences a GCC
    // -Warray-bounds false positive (a leaf-sized allocation reached through the
    // downcast looks like an out-of-bounds children access on an unreachable path).
    static btree_detail::InlineVec<Node*, Order + 1>& ch(Node* n) noexcept {
#if defined(__GNUC__)
        if (n->is_leaf) { __builtin_unreachable(); }
#endif
        return static_cast<InternalNode*>(n)->children;
    }

    // In-node position search. For arithmetic keys a branchless counting scan
    // (idx += comparison) avoids the data-dependent, mispredict-prone branches
    // of std::lower_bound/upper_bound and streams the contiguous inline keys
    // (GCC can vectorize it); for other key types we keep the binary search,
    // which does fewer of the potentially-expensive comparisons. The two
    // predicates are kept distinct (lower: k < key; upper: k <= key) to preserve
    // multiset duplicate placement exactly as std::lower_bound/upper_bound did.
    static size_t lower_index(const T* keys, size_t n, const T& key) {
        if constexpr (std::is_arithmetic_v<T>) {
            // O(1) boundary fast-paths: an append (key past the max) or prepend
            // (key at/below the min) needs no scan. This keeps sequential/sorted
            // insertion — where every key lands at the end — cheap, which the
            // full linear scan would otherwise make O(Order) per node. Both
            // branches are highly predictable, so the random path (which falls
            // through to the mispredict-free scan) is unaffected.
            if (n == 0 || key <= keys[0]) {
                return 0;
            }
            if (keys[n - 1] < key) {
                return n;
            }
            size_t idx = 0;
            for (size_t k = 0; k < n; ++k) {
                idx += (keys[k] < key);
            }
            return idx;
        } else {
            return static_cast<size_t>(std::lower_bound(keys, keys + n, key) - keys);
        }
    }

    static size_t upper_index(const T* keys, size_t n, const T& key) {
        if constexpr (std::is_arithmetic_v<T>) {
            if (n == 0 || key < keys[0]) {
                return 0;
            }
            if (keys[n - 1] <= key) {
                return n;
            }
            size_t idx = 0;
            for (size_t k = 0; k < n; ++k) {
                idx += (keys[k] <= key);
            }
            return idx;
        } else {
            return static_cast<size_t>(std::upper_bound(keys, keys + n, key) - keys);
        }
    }

    // Allocate a leaf or internal node of the right concrete type from the
    // matching size-class pool, then construct it in place.
    Node* make_node(bool leaf) {
        if (leaf) {
            return ::new (leaf_pool_.allocate()) Node(true);
        }
        return static_cast<Node*>(::new (internal_pool_.allocate()) InternalNode());
    }

    // Destroy a single node (running its true-type destructor) and return its
    // block to the matching pool for reuse. Does NOT recurse into children.
    void free_one(Node* n) noexcept {
        if (n->is_leaf) {
            n->~Node();
            leaf_pool_.deallocate(n);
        } else {
            InternalNode* in = static_cast<InternalNode*>(n);
            in->~InternalNode();
            internal_pool_.deallocate(in);
        }
    }

    // Run every node's destructor across a subtree (post-order). Used only on
    // teardown, where the pools are released wholesale afterwards, so blocks are
    // not returned to the free-list individually. Skipped entirely when T (hence
    // the node types) is trivially destructible.
    void destroy_subtree_dtors(Node* n) noexcept {
        if (!n->is_leaf) {
            InternalNode* in = static_cast<InternalNode*>(n);
            for (Node* c : in->children) {
                destroy_subtree_dtors(c);
            }
            in->~InternalNode();
        } else {
            n->~Node();
        }
    }

    // Destroy all nodes then release both pools. A node's only non-trivial state
    // is its keys, so when T is trivially destructible the per-node walk is a
    // no-op and is elided — teardown becomes O(#slabs) instead of O(#nodes).
    void destroy_all_nodes() noexcept {
        if (root != nullptr) {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                destroy_subtree_dtors(root);
            }
        }
        leaf_pool_.release();
        internal_pool_.release();
    }

    // Collapse an empty root after a removal: an internal root with no keys has
    // exactly one child, which becomes the new root; a leaf root becomes empty.
    // Kept out-of-line (noinline) so the optimizer analyses it with root of
    // unknown leaf-ness — inlining it into single-leaf-tree call sites otherwise
    // produces a -Warray-bounds false positive on the internal-root branch.
#if defined(__GNUC__)
    __attribute__((noinline))
#endif
    void shrink_root() noexcept {
        Node* old_root = root;
        if (root->is_leaf) {
            root = nullptr;
        } else {
            root = ch(root)[0];  // promote the sole child (its subtree stays live)
        }
        free_one(old_root);  // frees only old_root; the promoted child stays live
        --height_;           // both branches drop the tree by exactly one level
    }

    // Size (in blocks) of each slab, aimed at ~128 KB per slab, min 16 blocks.
    static constexpr std::size_t blocks_per_slab(std::size_t block_size) {
        std::size_t n = (128u * 1024u) / block_size;
        return n < 16 ? 16 : n;
    }

    // Round a node's size up to a block size that is >= sizeof(void*) (free-list
    // link) and a multiple of the block alignment.
    static constexpr std::size_t block_size_for(std::size_t sz, std::size_t align) {
        std::size_t bs = sz < sizeof(void*) ? sizeof(void*) : sz;
        return (bs + align - 1) / align * align;
    }

    static constexpr std::size_t leaf_align =
        alignof(Node) < alignof(void*) ? alignof(void*) : alignof(Node);
    static constexpr std::size_t internal_align =
        alignof(InternalNode) < alignof(void*) ? alignof(void*) : alignof(InternalNode);
    static constexpr std::size_t leaf_block = block_size_for(sizeof(Node), leaf_align);
    static constexpr std::size_t internal_block = block_size_for(sizeof(InternalNode), internal_align);

    void configure_pools() noexcept {
        leaf_pool_.configure(leaf_block, leaf_align, blocks_per_slab(leaf_block));
        internal_pool_.configure(internal_block, internal_align, blocks_per_slab(internal_block));
    }

    Node* root;
    size_t size_;
    // Tree height in levels (0 when empty), maintained incrementally so insert()
    // can size its pre-split node reservation without an extra root-to-leaf
    // descent: it grows by 1 when a new root is created and shrinks by 1 when the
    // root collapses. height() returns it directly.
    size_t height_;
    btree_detail::SlabPool leaf_pool_;      // blocks sized for Node
    btree_detail::SlabPool internal_pool_;  // blocks sized for InternalNode
    static constexpr int max_keys = Order - 1;
    static constexpr int min_keys = (Order - 1) / 2;

    // A lookup only ever compares keys with operator< (lower_index /
    // std::lower_bound) and operator== (the exact-match check); both must exist
    // for any usable key type, so this expression is well-formed for every T.
    // It is true for arithmetic keys and std::string (their comparisons are
    // noexcept) and false only when a user key type's comparison can throw. Used
    // to give search()/contains() a conditional noexcept: honestly noexcept in
    // the common case, without promising noexcept for throwing comparators (a
    // thrown comparison from a noexcept function would otherwise call terminate).
    static constexpr bool nothrow_search =
        noexcept(std::declval<const T&>() < std::declval<const T&>()) &&
        noexcept(std::declval<const T&>() == std::declval<const T&>());

public:
    // Forward iterator for in-order traversal
    class iterator {
    private:
        struct StackFrame {
            Node* node;
            size_t index;  // Next key index to visit
        };

        // Allocation-free path stack. Tree height is O(log n): the inline
        // capacity covers an order-3 tree up to ~2^31 elements without touching
        // the heap; deeper trees spill to a vector, preserving the documented
        // "billions of elements" contract. Making the common case allocation-free
        // removes the per-find()/per-begin() malloc+free.
        class PathStack {
            static constexpr size_t kInline = 32;
            StackFrame inline_[kInline];
            std::vector<StackFrame> heap_;  // used only after spilling
            size_t depth_ = 0;
            bool spilled_ = false;

        public:
            bool empty() const noexcept { return depth_ == 0; }

            void push(const StackFrame& f) {
                if (!spilled_ && depth_ < kInline) {
                    inline_[depth_++] = f;
                    return;
                }
                if (!spilled_) {  // inline buffer full: move everything to the heap
                    heap_.assign(inline_, inline_ + kInline);
                    spilled_ = true;
                }
                heap_.push_back(f);
                ++depth_;
            }

            StackFrame& top() noexcept {
                return spilled_ ? heap_.back() : inline_[depth_ - 1];
            }

            void pop() noexcept {
                --depth_;
                if (spilled_) {
                    heap_.pop_back();
                }
            }
        };

        PathStack stack_;
        const T* current_;

        void push_left_path(Node* node) {
            while (node != nullptr) {
                stack_.push({node, 0});
                if (node->is_leaf) {
                    break;
                }
                node = BTree::ch(node)[0];
            }
        }

        void advance() noexcept {
            while (!stack_.empty()) {
                StackFrame& frame = stack_.top();

                if (frame.index < frame.node->keys.size()) {
                    current_ = &frame.node->keys[frame.index];
                    frame.index++;

                    // If not a leaf, go to right child of current key
                    if (!frame.node->is_leaf && frame.index < BTree::ch(frame.node).size()) {
                        Node* right_child = BTree::ch(frame.node)[frame.index];
                        push_left_path(right_child);
                    }
                    return;
                } else {
                    // Done with this node, go back up
                    stack_.pop();
                }
            }
            current_ = nullptr;
        }

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        iterator() noexcept : current_(nullptr) {
            // Default constructor for end() - no stack needed
        }

        explicit iterator(Node* root) : current_(nullptr) {
            if (root != nullptr) {
                push_left_path(root);
                advance();
            }
        }

        reference operator*() const noexcept { return *current_; }
        pointer operator->() const noexcept { return current_; }

        iterator& operator++() noexcept {
            advance();
            return *this;
        }

        iterator operator++(int) noexcept {
            iterator tmp = *this;
            advance();
            return tmp;
        }

        bool operator==(const iterator& other) const noexcept {
            return current_ == other.current_;
        }

        bool operator!=(const iterator& other) const noexcept {
            return current_ != other.current_;
        }

        friend class BTree;  // Allow BTree to construct iterators with specific state
    };

    using const_iterator = iterator;  // All iterators are const (keys are immutable)

private:

    // Split an overflowed child (parent->children[index] holds max_keys+1 keys).
    // The median key moves up into the parent; the left half keeps `mid` keys and
    // the new right node gets the rest. Valid for every order: left ends with
    // floor(Order/2) keys and right with exactly min_keys keys.
    void split_child(Node* parent, size_t index) {
        Node* full_child = ch(parent)[index];
        Node* new_node = make_node(full_child->is_leaf);

        size_t mid = full_child->keys.size() / 2;
        T mid_key = std::move(full_child->keys[mid]);  // median moves up to the parent

        // Relocate the upper half (mid+1 .. end) into the new right node.
        new_node->keys.append_move(full_child->keys.raw() + mid + 1,
                                   full_child->keys.size() - (mid + 1));
        if (!full_child->is_leaf) {
            ch(new_node).append_move(ch(full_child).raw() + mid + 1,
                                     ch(full_child).size() - (mid + 1));
        }

        // Drop the median (now moved-from) and the relocated tail from the left.
        full_child->keys.resize(mid);
        if (!full_child->is_leaf) {
            ch(full_child).resize(mid + 1);
        }

        parent->keys.insert(parent->keys.begin() + index, std::move(mid_key));
        ch(parent).insert(ch(parent).begin() + index + 1, new_node);
    }

    // Reactive insert: descend to the target leaf and insert there, then split
    // the child on the way back up if it overflowed past max_keys.
    //
    // Templated on the key reference so a single descent serves both public
    // overloads: an lvalue key (K = const T&) is copied into its leaf slot, an
    // rvalue key (K = T) is moved. `key` is only ever *compared* (an lvalue use)
    // at each internal level and then forwarded unchanged to the next level, so
    // it is not relocated until the single leaf insert at the bottom of the
    // descent -- forwarding down the spine never moves it prematurely.
    template <typename K>
    void insert_rec(Node* node, K&& key) {
        if (node->is_leaf) {
            // Search for insertion position (duplicates allowed).
            size_t i = lower_index(node->keys.begin(), node->keys.size(), key);
            node->keys.insert(node->keys.begin() + i, std::forward<K>(key));
            return;
        }
        // upper_index places a new duplicate after existing equal keys.
        size_t i = upper_index(node->keys.begin(), node->keys.size(), key);

        insert_rec(ch(node)[i], std::forward<K>(key));

        if (ch(node)[i]->keys.size() > static_cast<size_t>(max_keys)) {
            split_child(node, i);
        }
    }

    // Shared body for the two public insert overloads (see insert()). Templated
    // on the key reference: an lvalue key is copied into its leaf, an rvalue key
    // is moved. Both share the same descent, split-cascade reservation, and
    // root-growth logic; only the final leaf placement differs (copy vs move).
    //
    // Strong exception guarantee (unchanged from the copy-only version): the only
    // steps that can throw -- the pre-commit pool reservations and, on the lvalue
    // path, the key copy that InlineVec::insert stages into a temporary before
    // touching the buffer -- all run before any node is mutated. A throwing
    // comparison during the descent likewise happens before the leaf is touched.
    // Once the key is placed at the leaf, every remaining step (buffer shuffles,
    // split relocations, root growth) is a noexcept move, so the mutation cannot
    // fail partway through. On the rvalue path the leaf placement is itself a
    // noexcept move, so that path cannot throw at all once the descent completes.
    template <typename K>
    void insert_impl(K&& key) {
        if (root == nullptr) {
            // Defer publishing `root` until the (possibly-throwing) placement of
            // `key` succeeds: if it throws, the tree must stay empty (root stays
            // null) rather than be left with a keyless root that makes empty()
            // disagree with size(). Free the node on the throwing path so it leaks
            // nothing.
            Node* n = make_node(true);
            try {
                n->keys.push_back(std::forward<K>(key));
            } catch (...) {
                free_one(n);
                throw;
            }
            root = n;
            size_++;
            height_ = 1;
            return;
        }

        // Pre-reserve every node the split cascade could allocate, BEFORE the key
        // is committed to a leaf. A single insert splits at most the one target
        // leaf (one new leaf) and, in the worst case, one node at every level up
        // to and including a freshly grown root (at most height()+1 new internal
        // nodes). Carving those blocks onto the pools' free-lists up front means
        // every make_node() on the recursion unwind reuses a block and never calls
        // ::operator new -- so once insert_rec() commits the key, no remaining step
        // can throw. If a reservation itself throws bad_alloc, the tree has not
        // been mutated yet, so insert()'s strong exception guarantee holds. Without
        // this, a bad_alloc from a split's make_node would leave a node holding
        // max_keys+1 keys -- an over-capacity, corrupt node -- with size_ uncounted.
        leaf_pool_.reserve(1);
        // Reserve internal blocks only when this insert could actually create one.
        // A single-leaf root grows an internal root only if the insert overflows
        // it; reserving otherwise would eagerly allocate an internal slab a small
        // tree never needs. An internal tree can cascade a split up every level
        // and grow a new root: at most height_ new internal nodes (+1 margin).
        if (!root->is_leaf) {
            internal_pool_.reserve(height_ + 1);
        } else if (root->keys.size() >= static_cast<size_t>(max_keys)) {
            internal_pool_.reserve(1);
        }

        insert_rec(root, std::forward<K>(key));

        // If the root overflowed, grow a new root above it and split.
        if (root->keys.size() > static_cast<size_t>(max_keys)) {
            Node* new_root = make_node(false);
            ch(new_root).push_back(root);
            split_child(new_root, 0);
            root = new_root;
            ++height_;
        }

        size_++;
    }

    // Bulk-load the tree from an already-sorted sequence of keys (moved out of
    // `keys`), building bottom-up so the whole structure is created with
    // sequential writes and no per-key root-to-leaf descent. Produces a valid,
    // balanced B-tree: leaves are packed to the fullest fill that still keeps
    // every node within [min_keys, max_keys] (the root may hold fewer, as the
    // invariants allow), so the tree is as shallow as possible.
    //
    // The keys interleave with the separators that live in parent nodes:
    //   [leaf 0 keys] s0 [leaf 1 keys] s1 ... s(L-2) [leaf L-1 keys]
    // so L leaves consume L-1 keys as separators; the same pattern repeats one
    // level up over the L leaves. Choosing L = ceil((n+1)/Order) guarantees each
    // leaf gets between min_keys and max_keys keys (and likewise every internal
    // node gets between min_keys+1 and max_keys+1 children), for every Order>=3.
    //
    // Precondition: the tree is empty and `keys` is sorted non-descending. The
    // only step here that can throw is a node allocation (bad_alloc); on any
    // throw every node allocated so far is freed and the tree is left empty, so
    // nothing leaks. Key moves are noexcept (BTree requires nothrow-move T).
    void build_from_sorted(std::vector<T> keys) {
        const std::size_t n = keys.size();
        if (n == 0) {
            return;  // tree stays empty
        }

        const std::size_t Ord = static_cast<std::size_t>(Order);
        // Minimal #leaves = ceil((n+1)/Order); the "+1" reserves the L-1 keys
        // that become separators in the parent level rather than leaf keys.
        std::size_t L = (n + Ord) / Ord;  // == ceil((n + 1) / Order)

        // Track every allocated node so a mid-build bad_alloc frees them all. The
        // finished tree has < 2*L nodes (each level shrinks by >= min_keys+1 >= 2),
        // so this reservation never grows and the push_back below cannot throw and
        // orphan a freshly allocated node.
        std::vector<Node*> allocated;
        allocated.reserve(2 * L + 16);

        auto alloc = [&](bool leaf) -> Node* {
            Node* nd = make_node(leaf);  // may throw bad_alloc
            allocated.push_back(nd);
            return nd;
        };

        try {
            // --- Leaf level: distribute n-(L-1) keys across L leaves, taking the
            // key that falls between two leaves as their separator. ---
            const std::size_t leaf_keys = n - (L - 1);
            const std::size_t base = leaf_keys / L;
            const std::size_t rem = leaf_keys % L;  // first `rem` leaves get +1

            std::vector<Node*> level;
            level.reserve(L);
            std::vector<T> seps;
            if (L > 1) seps.reserve(L - 1);

            std::size_t pos = 0;
            for (std::size_t li = 0; li < L; ++li) {
                std::size_t g = base + (li < rem ? 1 : 0);
                Node* leaf = alloc(true);
                for (std::size_t k = 0; k < g; ++k) {
                    leaf->keys.push_back(std::move(keys[pos++]));
                }
                level.push_back(leaf);
                if (li + 1 < L) {
                    seps.push_back(std::move(keys[pos++]));
                }
            }

            std::size_t h = 1;  // levels built so far (leaves are level 1)

            // --- Internal levels: group M children (with M-1 separators) into
            // P = ceil(M/Order) parents. A parent with c children stores its c-1
            // interior separators as keys; one further separator between adjacent
            // parents is promoted to the next round. ---
            while (level.size() > 1) {
                const std::size_t M = level.size();
                const std::size_t P = (M + Ord - 1) / Ord;  // ceil(M / Order)
                const std::size_t cbase = M / P;
                const std::size_t crem = M % P;  // first `crem` parents get +1 child

                std::vector<Node*> parent;
                parent.reserve(P);
                std::vector<T> psep;
                if (P > 1) psep.reserve(P - 1);

                std::size_t ci = 0;  // index into level (children)
                std::size_t si = 0;  // index into seps
                for (std::size_t p = 0; p < P; ++p) {
                    std::size_t c = cbase + (p < crem ? 1 : 0);
                    Node* in = alloc(false);
                    for (std::size_t k = 0; k < c; ++k) {
                        ch(in).push_back(level[ci++]);
                        if (k + 1 < c) {
                            in->keys.push_back(std::move(seps[si++]));
                        }
                    }
                    parent.push_back(in);
                    if (p + 1 < P) {
                        psep.push_back(std::move(seps[si++]));  // promote separator
                    }
                }

                level = std::move(parent);
                seps = std::move(psep);
                ++h;
            }

            root = level[0];
            size_ = n;
            height_ = h;
        } catch (...) {
            // free_one runs each node's true-type destructor (releasing any
            // moved-in separator keys) and returns the block to its pool. Child
            // pointers are not followed, so freeing every allocated node exactly
            // once is correct and cannot double-free.
            for (Node* nd : allocated) {
                free_one(nd);
            }
            root = nullptr;
            size_ = 0;
            height_ = 0;
            throw;
        }
    }

    Node* search_node(Node* node, const T& key) const {
        // Search for key position
        size_t i = lower_index(node->keys.begin(), node->keys.size(), key);

        if (i < node->keys.size() && node->keys[i] == key) {
            return node;
        }

        if (node->is_leaf) {
            return nullptr;
        }

        return search_node(ch(node)[i], key);
    }

    void traverse_node(Node* node) const {
        size_t i;
        for (i = 0; i < node->keys.size(); i++) {
            if (!node->is_leaf) {
                traverse_node(ch(node)[i]);
            }
            std::cout << node->keys[i] << " ";
        }

        if (!node->is_leaf) {
            traverse_node(ch(node)[i]);
        }
    }

    void traverse_node(Node* node, std::ostream& os) const {
        size_t i;
        for (i = 0; i < node->keys.size(); i++) {
            if (!node->is_leaf) {
                traverse_node(ch(node)[i], os);
            }
            os << node->keys[i] << " ";
        }

        if (!node->is_leaf) {
            traverse_node(ch(node)[i], os);
        }
    }

    template<typename Func>
    void for_each_node(Node* node, Func& f) const {
        size_t i;
        for (i = 0; i < node->keys.size(); i++) {
            if (!node->is_leaf) {
                for_each_node(ch(node)[i], f);
            }
            f(node->keys[i]);
        }

        if (!node->is_leaf) {
            for_each_node(ch(node)[i], f);
        }
    }

    const T& get_predecessor(Node* node) const {
        while (!node->is_leaf) {
            node = ch(node).back();
        }
        return node->keys.back();
    }

    const T& get_successor(Node* node) const {
        while (!node->is_leaf) {
            node = ch(node)[0];
        }
        return node->keys[0];
    }

    void merge_children(Node* node, size_t idx) {
        Node* left = ch(node)[idx];
        Node* right = ch(node)[idx + 1];

        // Bring the separator down, then relocate all of right's contents onto
        // the end of left (append_move memcpy's for trivial T, moves otherwise).
        left->keys.push_back(std::move(node->keys[idx]));
        left->keys.append_move(right->keys.raw(), right->keys.size());

        if (!left->is_leaf) {
            ch(left).append_move(ch(right).raw(), ch(right).size());
            ch(right).clear();  // detach transferred children so they aren't freed
        }

        // Remove key from parent
        node->keys.erase(node->keys.begin() + idx);
        ch(node).erase(ch(node).begin() + idx + 1);

        // Free the now-empty right node (its children, if any, moved to left)
        free_one(right);
        // No overflow is possible: reactive delete only merges an underflowed
        // child (min_keys-1 keys) with a minimum sibling (min_keys keys), giving
        // 2*min_keys <= max_keys keys for every order.
    }

    // Restore the min-keys invariant for children[idx] after a deletion may have
    // left it with fewer than min_keys keys: borrow from a sibling that can
    // spare a key, otherwise merge with an adjacent sibling.
    void fix_underflow(Node* node, size_t idx) {
        if (ch(node)[idx]->keys.size() >= static_cast<size_t>(min_keys)) {
            return;  // no underflow
        }
        // Try to borrow from left sibling
        if (idx > 0 && ch(node)[idx - 1]->keys.size() > static_cast<size_t>(min_keys)) {
            borrow_from_prev(node, idx);
        }
        // Try to borrow from right sibling
        else if (idx < ch(node).size() - 1 &&
                 ch(node)[idx + 1]->keys.size() > static_cast<size_t>(min_keys)) {
            borrow_from_next(node, idx);
        }
        // Merge with a sibling
        else {
            if (idx < ch(node).size() - 1) {
                merge_children(node, idx);       // merge with right sibling
            } else {
                merge_children(node, idx - 1);   // rightmost child: merge into left
            }
        }
    }

    // Right-rotate through the parent: the separator drops into the front of the
    // underful child and the left sibling's last key rises to the separator. All
    // key relocation is by move (noexcept for a nothrow-move T), so this repair
    // runs on the delete unwind without any throwing step — see remove()'s
    // strong-guarantee note.
    void borrow_from_prev(Node* node, size_t idx) {
        Node* child = ch(node)[idx];
        Node* sibling = ch(node)[idx - 1];

        // Separator moves down to the front of child; sibling's last key moves up.
        child->keys.insert(child->keys.begin(), std::move(node->keys[idx - 1]));
        node->keys[idx - 1] = std::move(sibling->keys.back());
        sibling->keys.pop_back();

        // Move child pointer if not leaf
        if (!child->is_leaf) {
            ch(child).insert(ch(child).begin(), ch(sibling).back());
            ch(sibling).pop_back();
        }
    }

    // Left-rotate through the parent: the separator drops onto the back of the
    // underful child and the right sibling's first key rises to the separator.
    // Move-only for the same exception-safety reason as borrow_from_prev.
    void borrow_from_next(Node* node, size_t idx) {
        Node* child = ch(node)[idx];
        Node* sibling = ch(node)[idx + 1];

        // Separator moves down to the back of child; sibling's first key moves up.
        child->keys.push_back(std::move(node->keys[idx]));
        node->keys[idx] = std::move(sibling->keys[0]);
        sibling->keys.erase(sibling->keys.begin());

        // Move child pointer if not leaf
        if (!child->is_leaf) {
            ch(child).push_back(ch(sibling)[0]);
            ch(sibling).erase(ch(sibling).begin());
        }
    }

    // Reactive delete: recurse to a leaf to perform the actual erase (swapping an
    // internal key with its in-leaf predecessor first), then repair any child that
    // underflowed on the way back up. Returns true if the key was found.
    bool remove_rec(Node* node, const T& key) {
        size_t idx = lower_index(node->keys.begin(), node->keys.size(), key);
        bool here = (idx < node->keys.size() && node->keys[idx] == key);

        if (node->is_leaf) {
            if (here) {
                node->keys.erase(node->keys.begin() + idx);
                return true;
            }
            return false;  // Key not found
        }

        size_t child_idx;
        bool removed;
        if (here) {
            // Internal key: replace it with its in-leaf predecessor (the rightmost
            // key of the left subtree), then delete that predecessor from the leaf.
            // Copy by value first (pre-commit: if this copy throws, nothing has
            // been mutated yet). Delete the predecessor from the subtree, THEN
            // overwrite the separator by *move* — that write lands after the leaf
            // erase (the sole commit point) and is noexcept, so it cannot break
            // remove()'s strong guarantee. The separator belongs to this node, not
            // the recursed subtree, so it is never read during the recursion;
            // leaving the old key in place until now yields the identical tree.
            T pred = get_predecessor(ch(node)[idx]);
            child_idx = idx;
            removed = remove_rec(ch(node)[child_idx], pred);
            node->keys[idx] = std::move(pred);
        } else {
            // lower_bound points at the subtree that may contain the key.
            child_idx = idx;
            removed = remove_rec(ch(node)[child_idx], key);
        }

        if (removed) {
            fix_underflow(node, child_idx);
        }
        return removed;
    }

    // Helper to find a key and build the iterator stack. Returns an iterator to
    // the FIRST (leftmost, in-order) element equal to `key`, or end() if absent.
    //
    // Returning the first equal element matters when duplicates are present: a
    // copy of `key` can live both at an internal separator and further left in
    // that separator's left subtree. Stopping at the first internal match (as a
    // naive descent would) yields a middle occurrence, so iterating
    // [find(key), end()) would silently skip the earlier duplicates and disagree
    // with std::find(begin(), end(), key). We therefore descend all the way to a
    // leaf along the lower_bound path, then take the deepest match on that path,
    // which is guaranteed to be the leftmost occurrence.
    iterator find_impl(const T& key) const {
        iterator result;  // empty stack, current_ == nullptr (i.e. end())
        if (root == nullptr) {
            return iterator();
        }

        // Descend to a leaf, pushing the lower_bound child index at each level.
        Node* node = root;
        while (true) {
            size_t i = lower_index(node->keys.begin(), node->keys.size(), key);
            result.stack_.push({node, i});
            if (node->is_leaf) {
                break;
            }
            node = ch(node)[i];
        }

        // Walk back up to the deepest frame whose key at its stored index equals
        // `key`. Every frame's index is its lower_bound position and we always
        // descended the left subtree, so any earlier duplicate would sit deeper
        // on this same path -- hence the deepest match is the first occurrence.
        while (!result.stack_.empty()) {
            auto& frame = result.stack_.top();
            Node* fn = frame.node;
            size_t i = frame.index;

            if (i < fn->keys.size() && fn->keys[i] == key) {
                result.current_ = &fn->keys[i];
                frame.index = i + 1;  // next in-order step resumes after this key

                // If internal, the successor is the left spine of the right
                // subtree; push it so ++ descends there first. (fn/i are locals,
                // so the pushes below can't be disturbed by a stack realloc.)
                if (!fn->is_leaf && i + 1 < ch(fn).size()) {
                    Node* right_child = ch(fn)[i + 1];
                    while (right_child != nullptr) {
                        result.stack_.push({right_child, 0});
                        if (right_child->is_leaf) break;
                        right_child = ch(right_child)[0];
                    }
                }
                return result;
            }
            result.stack_.pop();
        }

        return iterator();  // key not present
    }

    // Build an iterator to the first in-order element that is >= key (upper ==
    // false, "lower_bound") or > key (upper == true, "upper_bound"), or end() if
    // no such element exists. Mirrors find_impl's descent: push the
    // lower/upper-index child at each level down to a leaf, then walk back up to
    // the deepest frame whose stored index is still within its node's keys --
    // that key is the bound. Because the descent always takes the leftmost
    // qualifying child, the bound is the leftmost such element, so it is correct
    // in the presence of duplicates (matching std::multiset semantics).
    iterator bound_impl(const T& key, bool upper) const {
        iterator result;
        if (root == nullptr) {
            return iterator();
        }

        Node* node = root;
        while (true) {
            size_t i = upper
                           ? upper_index(node->keys.begin(), node->keys.size(), key)
                           : lower_index(node->keys.begin(), node->keys.size(), key);
            result.stack_.push({node, i});
            if (node->is_leaf) {
                break;
            }
            node = ch(node)[i];
        }

        // Pop frames whose index ran past the node's keys (the bound is neither in
        // nor at that node); the first frame with an in-range index holds it. The
        // exhausted subtree we descended contained only elements strictly on the
        // near side of the bound, so skipping it is correct.
        while (!result.stack_.empty()) {
            auto& frame = result.stack_.top();
            Node* fn = frame.node;
            size_t i = frame.index;

            if (i < fn->keys.size()) {
                result.current_ = &fn->keys[i];
                frame.index = i + 1;  // next in-order step resumes after this key

                // If internal, the successor is the left spine of the right
                // subtree; push it so ++ descends there first. (fn/i are locals,
                // so the pushes below can't be disturbed by a stack realloc.)
                if (!fn->is_leaf && i + 1 < ch(fn).size()) {
                    Node* right_child = ch(fn)[i + 1];
                    while (right_child != nullptr) {
                        result.stack_.push({right_child, 0});
                        if (right_child->is_leaf) break;
                        right_child = ch(right_child)[0];
                    }
                }
                return result;
            }
            result.stack_.pop();
        }

        return iterator();  // no element on the far side of the bound
    }

public:
    BTree() : root(nullptr), size_(0), height_(0) {
        configure_pools();
    }

    ~BTree() {
        destroy_all_nodes();
    }

    // Prevent copying (would cause double-free)
    BTree(const BTree&) = delete;
    BTree& operator=(const BTree&) = delete;

    // Move constructor - the node pools move with the tree so the moved-to tree
    // owns the storage its nodes live in.
    BTree(BTree&& other) noexcept
        : root(other.root),
          size_(other.size_),
          height_(other.height_),
          leaf_pool_(std::move(other.leaf_pool_)),
          internal_pool_(std::move(other.internal_pool_)) {
        other.root = nullptr;
        other.size_ = 0;
        other.height_ = 0;
    }

    // Move assignment
    BTree& operator=(BTree&& other) noexcept {
        if (this != &other) {
            destroy_all_nodes();
            root = other.root;
            size_ = other.size_;
            height_ = other.height_;
            leaf_pool_ = std::move(other.leaf_pool_);
            internal_pool_ = std::move(other.internal_pool_);
            other.root = nullptr;
            other.size_ = 0;
            other.height_ = 0;
        }
        return *this;
    }

    // Bulk-load constructor: build the tree from the range [first, last) in one
    // bottom-up pass -- sort the keys once (cache-friendly), then assemble the
    // leaves and internal levels with sequential writes -- instead of performing
    // `last - first` separate O(log n) inserts, each of which chases a fresh
    // root-to-leaf path. This makes constructing a tree from existing data
    // substantially faster (see the benchmark's "bulk-load" row). Duplicate keys
    // are supported (multiset semantics). Enabled only for genuine input
    // iterators (SFINAE) so it never hijacks other constructor calls.
    template <typename InputIt,
              typename = std::enable_if_t<std::is_convertible_v<
                  typename std::iterator_traits<InputIt>::iterator_category,
                  std::input_iterator_tag>>>
    BTree(InputIt first, InputIt last) : BTree() {
        std::vector<T> buf(first, last);  // gather (may throw: tree still empty)
        // Skip the sort when the input already arrives sorted (a common case:
        // loading from a sorted file/column/prior tree). is_sorted is a single
        // O(n) pass that bails at the first out-of-order pair, so it costs
        // essentially nothing on unsorted input but turns an already-sorted
        // bulk-load into a pure O(n) build.
        if (!std::is_sorted(buf.begin(), buf.end())) {
            std::sort(buf.begin(), buf.end());  // may throw: tree still empty
        }
        build_from_sorted(std::move(buf));
    }

    // Bulk-load from a braced list, e.g. BTree<int> t{3, 1, 4, 1, 5};
    BTree(std::initializer_list<T> init) : BTree(init.begin(), init.end()) {}

    // O(log n) - Insert a key into the tree by copying it.
    void insert(const T& key) { insert_impl(key); }

    // O(log n) - Insert a key into the tree by moving from `key`. For movable key
    // types (e.g. std::string) this relocates the key into its leaf slot instead
    // of copying it -- one fewer key copy per insert. `key` is left in a valid
    // moved-from state. Same strong exception guarantee as the copy overload
    // (see insert_impl); because T is required to be nothrow-move-constructible,
    // the actual relocation cannot throw.
    void insert(T&& key) { insert_impl(std::move(key)); }

    // O(log n) - Remove a key from the tree. Returns true if key was found and removed.
    bool remove(const T& key) {
        if (root == nullptr) {
            return false;
        }

        bool removed = remove_rec(root, key);

        if (removed) {
            size_--;
            // If root has no keys left, promote its child (internal) or empty
            // the tree (leaf). Handled in an out-of-line helper (see comment).
            if (root->keys.empty()) {
                shrink_root();
            }
        }

        return removed;
    }

    // O(log n) - Check if a key exists in the tree. noexcept iff T's comparisons
    // are noexcept (always so for arithmetic keys and std::string).
    [[nodiscard]] bool search(const T& key) const noexcept(nothrow_search) {
        if (root == nullptr) {
            return false;
        }
        return search_node(root, key) != nullptr;
    }

    // O(log n) - Alias for search()
    [[nodiscard]] bool contains(const T& key) const noexcept(nothrow_search) {
        return search(key);
    }

    // O(log n) lookup returning iterator to element, or end() if not found
    [[nodiscard]] iterator find(const T& key) const {
        return find_impl(key);
    }

    // O(log n) - Iterator to the first element not less than `key` (>= key), or
    // end() if every element is less than `key`. Matches std::multiset semantics.
    [[nodiscard]] iterator lower_bound(const T& key) const {
        return bound_impl(key, /*upper=*/false);
    }

    // O(log n) - Iterator to the first element greater than `key` (> key), or
    // end() if no element is greater. Matches std::multiset semantics.
    [[nodiscard]] iterator upper_bound(const T& key) const {
        return bound_impl(key, /*upper=*/true);
    }

    // O(log n) - The half-open range [lower_bound(key), upper_bound(key)) that
    // spans exactly the occurrences of `key`. Iterating it yields count(key)
    // elements, all equal to `key` (empty range if `key` is absent).
    [[nodiscard]] std::pair<iterator, iterator> equal_range(const T& key) const {
        return {bound_impl(key, /*upper=*/false), bound_impl(key, /*upper=*/true)};
    }

    // O(log n + k) - Number of occurrences of `key` (k = that count). Two
    // logarithmic descents to the range endpoints plus a walk across them --
    // far cheaper than an O(n) std::count over the whole tree.
    [[nodiscard]] size_t count(const T& key) const {
        iterator lo = bound_impl(key, /*upper=*/false);
        iterator hi = bound_impl(key, /*upper=*/true);
        size_t c = 0;
        for (; lo != hi; ++lo) {
            ++c;
        }
        return c;
    }

    // O(n) - Print all keys in sorted order to stdout
    void traverse() const {
        if (root != nullptr) {
            traverse_node(root);
            std::cout << std::endl;
        }
    }

    // O(n) - Print all keys in sorted order to the given stream
    void traverse(std::ostream& os) const {
        if (root != nullptr) {
            traverse_node(root, os);
            os << std::endl;
        }
    }

    // O(1) - Check if the tree is empty
    [[nodiscard]] bool empty() const noexcept {
        return root == nullptr;
    }

    // O(1) - Return the number of elements in the tree
    [[nodiscard]] size_t size() const noexcept {
        return size_;
    }

    // O(n) - Remove all elements from the tree
    void clear() noexcept {
        destroy_all_nodes();
        root = nullptr;
        size_ = 0;
        height_ = 0;
    }

    // O(1) - Return the height of the tree (0 for empty tree)
    [[nodiscard]] size_t height() const noexcept {
        return height_;
    }

    // O(log n) - Return the minimum element. Throws if tree is empty.
    [[nodiscard]] const T& min() const {
        if (root == nullptr) {
            throw std::runtime_error("min() called on empty tree");
        }
        Node* node = root;
        while (!node->is_leaf) {
            node = ch(node)[0];
        }
        return node->keys[0];
    }

    // O(log n) - Return the maximum element. Throws if tree is empty.
    [[nodiscard]] const T& max() const {
        if (root == nullptr) {
            throw std::runtime_error("max() called on empty tree");
        }
        Node* node = root;
        while (!node->is_leaf) {
            node = ch(node).back();
        }
        return node->keys.back();
    }

    // O(n) - Apply a function to each element in sorted order
    template<typename Func>
    void for_each(Func f) const {
        if (root != nullptr) {
            for_each_node(root, f);
        }
    }

    // O(n) - Return all elements as a sorted vector
    [[nodiscard]] std::vector<T> to_vector() const {
        std::vector<T> result;
        result.reserve(size_);
        for_each([&result](const T& key) {
            result.push_back(key);
        });
        return result;
    }

    // Iterator support - O(log n) for begin(), O(1) for end()
    // Iterator increment is amortized O(1)
    [[nodiscard]] iterator begin() const noexcept {
        return iterator(root);
    }

    [[nodiscard]] iterator end() const noexcept {
        return iterator();
    }

    [[nodiscard]] const_iterator cbegin() const noexcept {
        return begin();
    }

    [[nodiscard]] const_iterator cend() const noexcept {
        return end();
    }
};
