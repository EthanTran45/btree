#include <iostream>
#include <vector>
#include <algorithm>

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
    static constexpr int max_keys = Order - 1;
    static constexpr int min_keys = (Order - 1) / 2;

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
            node->keys.pop_back();
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

public:
    BTree() : root(nullptr) {}

    ~BTree() {
        delete root;
    }

    void insert(const T& key) {
        if (root == nullptr) {
            root = new Node(true);
            root->keys.push_back(key);
            return;
        }

        if (static_cast<int>(root->keys.size()) == max_keys) {
            Node* new_root = new Node(false);
            new_root->children.push_back(root);
            split_child(new_root, 0);
            root = new_root;
        }

        insert_non_full(root, key);
    }

    bool search(const T& key) const {
        if (root == nullptr) {
            return false;
        }
        return search_node(root, key) != nullptr;
    }

    void traverse() const {
        if (root != nullptr) {
            traverse_node(root);
            std::cout << std::endl;
        }
    }

    bool empty() const {
        return root == nullptr;
    }
};
