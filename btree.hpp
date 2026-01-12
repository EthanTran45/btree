#pragma once

#include <iostream>
#include <vector>
#include <stack>
#include <stdexcept>
#include <functional>
#include <algorithm>

// B-tree implementation with configurable order.
//
// Note: Order 3 has a known issue with remove() for certain random deletion
// patterns. For production use, Order >= 4 is recommended.
//
// Iterator Invalidation:
// - insert(): Invalidates all iterators (may cause node splits/reallocations)
// - remove(): Invalidates all iterators (may cause node merges/reallocations)
// - clear(): Invalidates all iterators
// - Iterators are safe to use only while the tree structure is unchanged.
// - Unlike std::map/std::set, ALL iterators are invalidated on any mutation.
template <typename T, int Order = 3>
class BTree {
private:
    struct Node {
        std::vector<T> keys;
        std::vector<Node*> children;
        bool is_leaf;

        Node(bool leaf = true) : is_leaf(leaf) {
            // Pre-allocate to avoid reallocations during node filling
            keys.reserve(Order - 1);  // max_keys
            if (!leaf) {
                children.reserve(Order);  // max_keys + 1
            }
        }

        ~Node() {
            for (Node* child : children) {
                delete child;
            }
        }
    };

    Node* root;
    size_t size_;
    static constexpr int max_keys = Order - 1;
    static constexpr int min_keys = (Order - 1) / 2;

public:
    // Forward iterator for in-order traversal
    class iterator {
    private:
        struct StackFrame {
            Node* node;
            size_t index;  // Next key index to visit
        };
        std::stack<StackFrame, std::vector<StackFrame>> stack_;  // Vector-backed for cache locality
        const T* current_;

        void push_left_path(Node* node) {
            while (node != nullptr) {
                stack_.push({node, 0});
                if (node->is_leaf) {
                    break;
                }
                node = node->children[0];
            }
        }

        void advance() noexcept {
            while (!stack_.empty()) {
                StackFrame& frame = stack_.top();

                if (frame.index < frame.node->keys.size()) {
                    current_ = &frame.node->keys[frame.index];
                    frame.index++;

                    // If not a leaf, go to right child of current key
                    if (!frame.node->is_leaf && frame.index < frame.node->children.size()) {
                        Node* right_child = frame.node->children[frame.index];
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
            // Pre-allocate stack for typical tree heights (handles billions of elements)
            std::vector<StackFrame> container;
            container.reserve(32);
            stack_ = std::stack<StackFrame, std::vector<StackFrame>>(std::move(container));

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

    void split_child(Node* parent, size_t index) {
        Node* full_child = parent->children[index];
        Node* new_node = new Node(full_child->is_leaf);

        size_t mid = max_keys / 2;
        T mid_key = full_child->keys[mid];

        new_node->keys.assign(full_child->keys.begin() + mid + 1, full_child->keys.end());
        full_child->keys.resize(mid);

        if (!full_child->is_leaf) {
            new_node->children.assign(full_child->children.begin() + mid + 1, full_child->children.end());
            full_child->children.resize(mid + 1);
        }

        parent->keys.insert(parent->keys.begin() + index, mid_key);
        parent->children.insert(parent->children.begin() + index + 1, new_node);
    }

    void insert_non_full(Node* node, const T& key) {
        if (node->is_leaf) {
            // Use binary search to find insertion position
            auto pos = std::lower_bound(node->keys.begin(), node->keys.end(), key);
            node->keys.insert(pos, key);
        } else {
            // Use binary search to find child
            auto pos = std::upper_bound(node->keys.begin(), node->keys.end(), key);
            size_t i = pos - node->keys.begin();

            if (node->children[i]->keys.size() == static_cast<size_t>(max_keys)) {
                split_child(node, i);
                if (key > node->keys[i]) {
                    i++;
                }
            }
            insert_non_full(node->children[i], key);
        }
    }

    Node* search_node(Node* node, const T& key) const {
        // Binary search for key position
        auto it = std::lower_bound(node->keys.begin(), node->keys.end(), key);
        size_t i = it - node->keys.begin();

        if (i < node->keys.size() && node->keys[i] == key) {
            return node;
        }

        if (node->is_leaf) {
            return nullptr;
        }

        return search_node(node->children[i], key);
    }

    void traverse_node(Node* node) const {
        size_t i;
        for (i = 0; i < node->keys.size(); i++) {
            if (!node->is_leaf) {
                traverse_node(node->children[i]);
            }
            std::cout << node->keys[i] << " ";
        }

        if (!node->is_leaf) {
            traverse_node(node->children[i]);
        }
    }

    void traverse_node(Node* node, std::ostream& os) const {
        size_t i;
        for (i = 0; i < node->keys.size(); i++) {
            if (!node->is_leaf) {
                traverse_node(node->children[i], os);
            }
            os << node->keys[i] << " ";
        }

        if (!node->is_leaf) {
            traverse_node(node->children[i], os);
        }
    }

    size_t calculate_height(Node* node) const noexcept {
        if (node == nullptr) {
            return 0;
        }
        if (node->is_leaf) {
            return 1;
        }
        return 1 + calculate_height(node->children[0]);
    }

    template<typename Func>
    void for_each_node(Node* node, Func& f) const {
        size_t i;
        for (i = 0; i < node->keys.size(); i++) {
            if (!node->is_leaf) {
                for_each_node(node->children[i], f);
            }
            f(node->keys[i]);
        }

        if (!node->is_leaf) {
            for_each_node(node->children[i], f);
        }
    }

    const T& get_predecessor(Node* node) const {
        while (!node->is_leaf) {
            node = node->children.back();
        }
        return node->keys.back();
    }

    const T& get_successor(Node* node) const {
        while (!node->is_leaf) {
            node = node->children[0];
        }
        return node->keys[0];
    }

    void merge_children(Node* node, size_t idx) {
        Node* left = node->children[idx];
        Node* right = node->children[idx + 1];

        // Reserve space and bulk-insert to avoid multiple reallocations
        left->keys.reserve(left->keys.size() + 1 + right->keys.size());
        left->keys.push_back(node->keys[idx]);
        left->keys.insert(left->keys.end(), right->keys.begin(), right->keys.end());

        if (!left->is_leaf) {
            left->children.reserve(left->children.size() + right->children.size());
            left->children.insert(left->children.end(), right->children.begin(), right->children.end());
        }

        // Remove key from parent
        node->keys.erase(node->keys.begin() + idx);
        node->children.erase(node->children.begin() + idx + 1);

        // Delete right node (but not its children, as they're now in left)
        right->children.clear();
        delete right;

        // For small orders (like 3), merging can cause overflow.
        // If so, split the merged node and push a key back to parent.
        if (left->keys.size() > static_cast<size_t>(max_keys)) {
            Node* new_node = new Node(left->is_leaf);
            // Use floor division for mid - ensures left gets at least floor(n/2) keys
            size_t total_keys = left->keys.size();
            size_t mid = total_keys / 2;

            // Ensure both halves have at least 1 key
            if (mid == 0) mid = 1;
            if (mid >= total_keys - 1) mid = total_keys - 2;

            T mid_key = left->keys[mid];

            new_node->keys.assign(left->keys.begin() + mid + 1, left->keys.end());
            left->keys.resize(mid);

            if (!left->is_leaf) {
                new_node->children.assign(left->children.begin() + mid + 1, left->children.end());
                left->children.resize(mid + 1);
            }

            // Insert middle key back into parent at the same position
            node->keys.insert(node->keys.begin() + idx, mid_key);
            node->children.insert(node->children.begin() + idx + 1, new_node);
        }
    }

    void fill_child(Node* node, size_t idx) {
        // Try to borrow from left sibling
        if (idx > 0 && node->children[idx - 1]->keys.size() > static_cast<size_t>(min_keys)) {
            borrow_from_prev(node, idx);
        }
        // Try to borrow from right sibling
        else if (idx < node->children.size() - 1 &&
                 node->children[idx + 1]->keys.size() > static_cast<size_t>(min_keys)) {
            borrow_from_next(node, idx);
        }
        // Merge with a sibling
        else {
            if (idx < node->children.size() - 1) {
                merge_children(node, idx);
            } else {
                merge_children(node, idx - 1);
            }
        }
    }

    void borrow_from_prev(Node* node, size_t idx) {
        Node* child = node->children[idx];
        Node* sibling = node->children[idx - 1];

        // Shift all keys in child one step ahead
        child->keys.insert(child->keys.begin(), node->keys[idx - 1]);

        // Move key from sibling to parent
        node->keys[idx - 1] = sibling->keys.back();
        sibling->keys.pop_back();

        // Move child pointer if not leaf
        if (!child->is_leaf) {
            child->children.insert(child->children.begin(), sibling->children.back());
            sibling->children.pop_back();
        }
    }

    void borrow_from_next(Node* node, size_t idx) {
        Node* child = node->children[idx];
        Node* sibling = node->children[idx + 1];

        // Move key from parent to child
        child->keys.push_back(node->keys[idx]);

        // Move key from sibling to parent
        node->keys[idx] = sibling->keys[0];
        sibling->keys.erase(sibling->keys.begin());

        // Move child pointer if not leaf
        if (!child->is_leaf) {
            child->children.push_back(sibling->children[0]);
            sibling->children.erase(sibling->children.begin());
        }
    }

    bool remove_from_node(Node* node, const T& key) {
        // Binary search for key position
        auto it = std::lower_bound(node->keys.begin(), node->keys.end(), key);
        size_t idx = it - node->keys.begin();

        // Key found in this node
        if (idx < node->keys.size() && node->keys[idx] == key) {
            if (node->is_leaf) {
                // Case 1: Key is in leaf node - simply remove it
                node->keys.erase(node->keys.begin() + idx);
                return true;
            } else {
                // Case 2: Key is in internal node
                // For Order 3, merging two min-key children causes overflow (3 keys > max 2).
                // Always use predecessor/successor approach to avoid this issue.
                if (node->children[idx]->keys.size() > static_cast<size_t>(min_keys)) {
                    // Case 2a: Left child has enough keys
                    T pred = get_predecessor(node->children[idx]);
                    node->keys[idx] = pred;
                    return remove_from_node(node->children[idx], pred);
                } else if (node->children[idx + 1]->keys.size() > static_cast<size_t>(min_keys)) {
                    // Case 2b: Right child has enough keys
                    T succ = get_successor(node->children[idx + 1]);
                    node->keys[idx] = succ;
                    return remove_from_node(node->children[idx + 1], succ);
                } else {
                    // Case 2c: Both children have minimum keys - merge them
                    // The key at node->keys[idx] gets pushed down to merged child.
                    // merge_children handles overflow by splitting if needed.
                    merge_children(node, idx);

                    // After merge (and possible split), find where the key ended up.
                    // Search for it in the current node first.
                    auto new_it = std::lower_bound(node->keys.begin(), node->keys.end(), key);
                    size_t new_idx = new_it - node->keys.begin();

                    if (new_idx < node->keys.size() && node->keys[new_idx] == key) {
                        // Key was pushed back up as the split middle - handle as internal node key
                        // Use Case 2a (predecessor) since left child should have enough keys after split
                        T pred = get_predecessor(node->children[new_idx]);
                        node->keys[new_idx] = pred;
                        return remove_from_node(node->children[new_idx], pred);
                    } else {
                        // Key is in one of the children
                        return remove_from_node(node->children[new_idx], key);
                    }
                }
            }
        } else {
            // Key not in this node
            if (node->is_leaf) {
                return false;  // Key not found
            }

            bool is_last = (idx == node->keys.size());

            // Ensure child has enough keys before descending
            if (node->children[idx]->keys.size() <= static_cast<size_t>(min_keys)) {
                fill_child(node, idx);
            }

            // After filling, the child at idx may have been merged with previous sibling
            if (is_last && idx > node->keys.size()) {
                return remove_from_node(node->children[idx - 1], key);
            } else {
                return remove_from_node(node->children[idx], key);
            }
        }
    }

    // Helper to find a key and build iterator stack
    iterator find_impl(const T& key) const {
        if (root == nullptr) {
            return iterator();
        }

        // Pre-allocate stack for typical tree heights
        std::vector<typename iterator::StackFrame> path_container;
        path_container.reserve(32);
        std::stack<typename iterator::StackFrame, std::vector<typename iterator::StackFrame>> path(std::move(path_container));
        Node* node = root;

        while (node != nullptr) {
            auto it = std::lower_bound(node->keys.begin(), node->keys.end(), key);
            size_t i = it - node->keys.begin();

            if (i < node->keys.size() && node->keys[i] == key) {
                // Found the key - build iterator at this position
                // Push current node with index pointing to the found key
                path.push({node, i});

                // Create iterator and set its state
                iterator result;
                result.stack_ = std::move(path);
                result.current_ = &node->keys[i];

                // Advance index for next iteration
                auto& frame = result.stack_.top();
                frame.index++;

                // If not a leaf, push left path of right subtree
                if (!node->is_leaf && frame.index < node->children.size()) {
                    Node* right_child = node->children[frame.index];
                    while (right_child != nullptr) {
                        result.stack_.push({right_child, 0});
                        if (right_child->is_leaf) break;
                        right_child = right_child->children[0];
                    }
                }

                return result;
            }

            if (node->is_leaf) {
                return iterator();  // Not found
            }

            path.push({node, i});
            node = node->children[i];
        }

        return iterator();
    }

public:
    BTree() : root(nullptr), size_(0) {}

    ~BTree() {
        delete root;
    }

    // Prevent copying (would cause double-free)
    BTree(const BTree&) = delete;
    BTree& operator=(const BTree&) = delete;

    // Move constructor
    BTree(BTree&& other) noexcept : root(other.root), size_(other.size_) {
        other.root = nullptr;
        other.size_ = 0;
    }

    // Move assignment
    BTree& operator=(BTree&& other) noexcept {
        if (this != &other) {
            delete root;
            root = other.root;
            size_ = other.size_;
            other.root = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    // O(log n) - Insert a key into the tree
    void insert(const T& key) {
        if (root == nullptr) {
            root = new Node(true);
            root->keys.push_back(key);
            size_++;
            return;
        }

        if (root->keys.size() == static_cast<size_t>(max_keys)) {
            Node* new_root = new Node(false);
            new_root->children.push_back(root);
            split_child(new_root, 0);
            root = new_root;
        }

        insert_non_full(root, key);
        size_++;
    }

    // O(log n) - Remove a key from the tree. Returns true if key was found and removed.
    bool remove(const T& key) {
        if (root == nullptr) {
            return false;
        }

        bool removed = remove_from_node(root, key);

        if (removed) {
            size_--;
            // If root has no keys left, make its first child the new root
            if (root->keys.empty()) {
                Node* old_root = root;
                if (root->is_leaf) {
                    root = nullptr;
                } else {
                    root = root->children[0];
                }
                old_root->children.clear();  // Prevent recursive deletion
                delete old_root;
            }
        }

        return removed;
    }

    // O(log n) - Check if a key exists in the tree
    [[nodiscard]] bool search(const T& key) const noexcept {
        if (root == nullptr) {
            return false;
        }
        return search_node(root, key) != nullptr;
    }

    // O(log n) - Alias for search()
    [[nodiscard]] bool contains(const T& key) const noexcept {
        return search(key);
    }

    // O(log n) lookup returning iterator to element, or end() if not found
    [[nodiscard]] iterator find(const T& key) const {
        return find_impl(key);
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
        delete root;
        root = nullptr;
        size_ = 0;
    }

    // O(log n) - Return the height of the tree (0 for empty tree)
    [[nodiscard]] size_t height() const noexcept {
        return calculate_height(root);
    }

    // O(log n) - Return the minimum element. Throws if tree is empty.
    [[nodiscard]] const T& min() const {
        if (root == nullptr) {
            throw std::runtime_error("min() called on empty tree");
        }
        Node* node = root;
        while (!node->is_leaf) {
            node = node->children[0];
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
            node = node->children.back();
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
