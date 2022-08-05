#include <iostream>
#include <cstdlib>

class Node {
public:
    Node(int x, int y):x(x), y(y), left_(nullptr), right_(nullptr) {}
    int x;
    int y;
    Node* left_;
    Node* right_;
};

class Treap {
public:
    Treap():root_(nullptr) {}
    void Insert(int key) {
        int value = std::rand();
        auto [t1, t2] = Split(root_, key);
        Node* temp = new Node(key, value);
        root_ = Merge(Merge(t1, temp), t2);
    }

    bool Contains(int key) {
        return Contains_(root_, key);
    }
private:
    bool Contains_(Node* node, int key) {
        if (!node) {return false;}
        if (node->x == key) {return true;}
        if (node->x < key) {return Contains_(node->right_, key);}
        return Contains_(node->left_, key);
    }
    std::pair<Node*, Node*> Split(Node* split_node_, int key) {
        if (!split_node_) return {nullptr, nullptr};

        if (key > split_node_->x) {
            auto [t1, t2] = Split(split_node_->right_, key);
            split_node_->right_ = t1;
            return {split_node_, t2};
        }
        auto [t1, t2] = Split(split_node_->left_, key);
        split_node_->left_ = t2;
        return {t1, split_node_};
    }

    Node* Merge (Node* first, Node* second) {
        if (!first) { return second;}
        if (!second) { return first;}
        if (first->y < second->y) {
            Node* left_node = Merge(first, second->left_);
            second->left_ = left_node;
            return second;
        }
        Node* right_node = Merge(first->right_, second);
        first->right_ = right_node;
        return first;
    }

    Node* root_;

};

int main() {
    std::srand(1);
    Treap tree;
    tree.Insert(5);
    tree.Insert(6);
    std::cout << tree.Contains(6);
    return 0;
}
