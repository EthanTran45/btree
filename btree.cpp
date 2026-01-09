#include <iostream>
#include <vector>
#include <stack>
#include <stdexcept>
#include <functional>

template <typename T, int Order = 3>
class BTree {
private:
    struct Node {
        std::vector<T> keys;
        std::vector<Node*> children;
        bool is_leaf;

        Node(bool leaf = true) : is_leaf(leaf) {}

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
            int index;  // Next key index to visit
        };
        std::stack<StackFrame> stack_;
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

        void advance() {
            if (stack_.empty()) {
                current_ = nullptr;
                return;
            }

            StackFrame& frame = stack_.top();

            if (frame.index < static_cast<int>(frame.node->keys.size())) {
                current_ = &frame.node->keys[frame.index];
                frame.index++;

                // If not a leaf, go to right child of current key
                if (!frame.node->is_leaf && frame.index < static_cast<int>(frame.node->children.size())) {
                    Node* right_child = frame.node->children[frame.index];
                    push_left_path(right_child);
                }
            } else {
                // Done with this node, go back up
                stack_.pop();
                advance();
            }
        }

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        iterator() : current_(nullptr) {}

        explicit iterator(Node* root) : current_(nullptr) {
            if (root != nullptr) {
                push_left_path(root);
                advance();
            }
        }

        reference operator*() const { return *current_; }
        pointer operator->() const { return current_; }

        iterator& operator++() {
            advance();
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            advance();
            return tmp;
        }

        bool operator==(const iterator& other) const {
            return current_ == other.current_;
        }

        bool operator!=(const iterator& other) const {
            return current_ != other.current_;
        }
    };

    using const_iterator = iterator;  // All iterators are const (keys are immutable)

private:

    void split_child(Node* parent, int index) {
        Node* full_child = parent->children[index];
        Node* new_node = new Node(full_child->is_leaf);

        int mid = max_keys / 2;
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
        int i = node->keys.size() - 1;

        if (node->is_leaf) {
            node->keys.push_back(key);
            while (i >= 0 && key < node->keys[i]) {
                node->keys[i + 1] = node->keys[i];
                i--;
            }
            node->keys[i + 1] = key;
        } else {
            while (i >= 0 && key < node->keys[i]) {
                i--;
            }
            i++;

            if (static_cast<int>(node->children[i]->keys.size()) == max_keys) {
                split_child(node, i);
                if (key > node->keys[i]) {
                    i++;
                }
            }
            insert_non_full(node->children[i], key);
        }
    }

    Node* search_node(Node* node, const T& key) const {
        int i = 0;
        while (i < static_cast<int>(node->keys.size()) && key > node->keys[i]) {
            i++;
        }

        if (i < static_cast<int>(node->keys.size()) && key == node->keys[i]) {
            return node;
        }

        if (node->is_leaf) {
            return nullptr;
        }

        return search_node(node->children[i], key);
    }

    void traverse_node(Node* node) const {
        int i;
        for (i = 0; i < static_cast<int>(node->keys.size()); i++) {
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
        int i;
        for (i = 0; i < static_cast<int>(node->keys.size()); i++) {
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
        int i;
        for (i = 0; i < static_cast<int>(node->keys.size()); i++) {
            if (!node->is_leaf) {
                for_each_node(node->children[i], f);
            }
            f(node->keys[i]);
        }

        if (!node->is_leaf) {
            for_each_node(node->children[i], f);
        }
    }

    T get_predecessor(Node* node) {
        while (!node->is_leaf) {
            node = node->children[node->children.size() - 1];
        }
        return node->keys[node->keys.size() - 1];
    }

    T get_successor(Node* node) {
        while (!node->is_leaf) {
            node = node->children[0];
        }
        return node->keys[0];
    }

    void merge_children(Node* node, int idx) {
        Node* left = node->children[idx];
        Node* right = node->children[idx + 1];

        // Move key from parent to left child
        left->keys.push_back(node->keys[idx]);

        // Move all keys from right to left
        for (const T& key : right->keys) {
            left->keys.push_back(key);
        }

        // Move all children from right to left
        for (Node* child : right->children) {
            left->children.push_back(child);
        }

        // Remove key from parent
        node->keys.erase(node->keys.begin() + idx);
        node->children.erase(node->children.begin() + idx + 1);

        // Delete right node (but not its children, as they're now in left)
        right->children.clear();
        delete right;
    }

    void fill_child(Node* node, int idx) {
        // Try to borrow from left sibling
        if (idx > 0 && static_cast<int>(node->children[idx - 1]->keys.size()) > min_keys) {
            borrow_from_prev(node, idx);
        }
        // Try to borrow from right sibling
        else if (idx < static_cast<int>(node->children.size()) - 1 &&
                 static_cast<int>(node->children[idx + 1]->keys.size()) > min_keys) {
            borrow_from_next(node, idx);
        }
        // Merge with a sibling
        else {
            if (idx < static_cast<int>(node->children.size()) - 1) {
                merge_children(node, idx);
            } else {
                merge_children(node, idx - 1);
            }
        }
    }

    void borrow_from_prev(Node* node, int idx) {
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

    void borrow_from_next(Node* node, int idx) {
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
        int idx = 0;
        while (idx < static_cast<int>(node->keys.size()) && node->keys[idx] < key) {
            idx++;
        }

        // Key found in this node
        if (idx < static_cast<int>(node->keys.size()) && node->keys[idx] == key) {
            if (node->is_leaf) {
                // Case 1: Key is in leaf node - simply remove it
                node->keys.erase(node->keys.begin() + idx);
                return true;
            } else {
                // Case 2: Key is in internal node
                if (static_cast<int>(node->children[idx]->keys.size()) > min_keys) {
                    // Case 2a: Left child has enough keys
                    T pred = get_predecessor(node->children[idx]);
                    node->keys[idx] = pred;
                    return remove_from_node(node->children[idx], pred);
                } else if (static_cast<int>(node->children[idx + 1]->keys.size()) > min_keys) {
                    // Case 2b: Right child has enough keys
                    T succ = get_successor(node->children[idx + 1]);
                    node->keys[idx] = succ;
                    return remove_from_node(node->children[idx + 1], succ);
                } else {
                    // Case 2c: Both children have minimum keys - merge them
                    merge_children(node, idx);
                    return remove_from_node(node->children[idx], key);
                }
            }
        } else {
            // Key not in this node
            if (node->is_leaf) {
                return false;  // Key not found
            }

            bool is_last = (idx == static_cast<int>(node->keys.size()));

            // Ensure child has enough keys before descending
            if (static_cast<int>(node->children[idx]->keys.size()) <= min_keys) {
                fill_child(node, idx);
            }

            // After filling, the child at idx may have been merged with previous sibling
            if (is_last && idx > static_cast<int>(node->keys.size())) {
                return remove_from_node(node->children[idx - 1], key);
            } else {
                return remove_from_node(node->children[idx], key);
            }
        }
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

    void insert(const T& key) {
        if (root == nullptr) {
            root = new Node(true);
            root->keys.push_back(key);
            size_++;
            return;
        }

        if (static_cast<int>(root->keys.size()) == max_keys) {
            Node* new_root = new Node(false);
            new_root->children.push_back(root);
            split_child(new_root, 0);
            root = new_root;
        }

        insert_non_full(root, key);
        size_++;
    }

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

    [[nodiscard]] bool search(const T& key) const noexcept {
        if (root == nullptr) {
            return false;
        }
        return search_node(root, key) != nullptr;
    }

    [[nodiscard]] bool contains(const T& key) const noexcept {
        return search(key);
    }

    void traverse() const {
        if (root != nullptr) {
            traverse_node(root);
            std::cout << std::endl;
        }
    }

    void traverse(std::ostream& os) const {
        if (root != nullptr) {
            traverse_node(root, os);
            os << std::endl;
        }
    }

    [[nodiscard]] bool empty() const noexcept {
        return root == nullptr;
    }

    [[nodiscard]] size_t size() const noexcept {
        return size_;
    }

    void clear() noexcept {
        delete root;
        root = nullptr;
        size_ = 0;
    }

    [[nodiscard]] size_t height() const noexcept {
        return calculate_height(root);
    }

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

    [[nodiscard]] const T& max() const {
        if (root == nullptr) {
            throw std::runtime_error("max() called on empty tree");
        }
        Node* node = root;
        while (!node->is_leaf) {
            node = node->children[node->children.size() - 1];
        }
        return node->keys[node->keys.size() - 1];
    }

    template<typename Func>
    void for_each(Func f) const {
        if (root != nullptr) {
            for_each_node(root, f);
        }
    }

    [[nodiscard]] std::vector<T> to_vector() const {
        std::vector<T> result;
        result.reserve(size_);
        for_each([&result](const T& key) {
            result.push_back(key);
        });
        return result;
    }

    // Iterator support
    [[nodiscard]] iterator begin() const {
        return iterator(root);
    }

    [[nodiscard]] iterator end() const {
        return iterator();
    }

    [[nodiscard]] const_iterator cbegin() const {
        return begin();
    }

    [[nodiscard]] const_iterator cend() const {
        return end();
    }
};
