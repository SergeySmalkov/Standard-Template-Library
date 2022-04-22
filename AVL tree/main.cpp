#include <iostream>
#include <memory>

template<typename Key>
struct Node {
    std::shared_ptr<Node<Key>> left{nullptr};
    std::shared_ptr<Node<Key>> right{nullptr};
    int64_t height{1};
    Key key;
    explicit Node(const Key& key):key(key) {}
};

template<typename Key, typename Comp = std::less<Key>, typename Allocator = std::allocator<Node<Key>>>
class AVLtree {
public:
    template<bool isConst>
    friend class iterator;

    template<bool isConst>
    class iterator {
    public:
        friend class AVLtree<Key>;

        iterator(std::shared_ptr<Node<Key>> node, AVLtree<Key>* avl): to_(node), avltree_(avl) {}

        iterator(const iterator<isConst>& other): to_(other.to_), avltree_(other.avltree_) {}

        iterator<isConst>& operator = (const iterator<isConst>& other) {
            if(this == std::addressof(other)) {
                return *this;
            }
            to_ = other.to_;
            avltree_ = other.avltree_;
            return *this;
        }

        iterator<isConst>& operator ++ () {
                if(to_ == nullptr) {
                    return *this;
                }
                if(to_ == avltree_->FindMax(avltree_->root_)) {
                    to_ = to_->right;
                    return *this;
                }
                if(to_->right != nullptr) {
                    to_ = avltree_->FindMin(to_->right);
                    return *this;
                }
                while (true) {
                    if (avltree_->FindRoot(to_)->left->key == to_->key) {
                        to_ = avltree_->FindRoot(to_);
                        return *this;
                    } else {
                        to_ = avltree_->FindRoot(to_);
                    }
                }
            }

        iterator<isConst> operator ++ (int) {
            auto copy = *this;
            if(to_ == avltree_->FindMax(avltree_->root_)) {
                return *this;
            }
            ++*this;
            return copy;
        }

        iterator<isConst>& operator -- () {
            if(to_ == nullptr) {
                return avltree_->FindMax(avltree_->root_);
            }
            if(to_ == avltree_->FindMin(avltree_->root_)) {
                return *this;
            }
            if(to_->left != nullptr) {
                return avltree_->FindMax(to_->left);
            }
            while (true) {
                if (avltree_->FindRoot(to_)->right->key == to_->key) {
                    return avltree_->FindRoot(to_);
                } else {
                    to_ = avltree_->FindRoot(to_);
                }
            }
        }

        iterator<isConst> operator -- (int) {
            auto copy = *this;
            --*this;
            return copy;
        }

        std::conditional_t<isConst, const Key&, Key&> operator * () {
            return to_->key;
        }

        std::conditional_t<isConst, const Key*, Key*> operator -> () {
            return std::addressof(to_->key);
        }

        bool operator == (const iterator& other) {
            return avltree_->root_ == other.avltree_->root_ && to_ == other.to_;
        }

        bool operator != (const iterator& other) {
            return avltree_->root_ != other.avltree_->root_ || to_ != other.to_;
        }

    private:
        std::shared_ptr<Node<Key>> to_;
        AVLtree<Key, Comp, Allocator>* avltree_;
    };

    using it = iterator<false>;
    using const_it = iterator<true>;

    it begin() {
        return it(FindMin(root_), this);
    }

    it end() {
        return it(FindMax(root_)->right, this);
    }

    const_it cbegin() {
        return const_it(FindMin(root_), this);
    }

    const_it cend() {
        return const_it(FindMax(root_)->right, this);
    }

    AVLtree():root_{nullptr} {}
    void Insert(const Key& key) {
        root_ = Insert_(root_, key);
    }

    void Delete(const Key key) {
        root_ = Delete_(root_, key);
    }

    bool Contains(const Key& key) {
        return Contains_(root_, key);
    }

    int64_t Height() {
        return Height_(root_);
    }

private:

    std::shared_ptr<Node<Key>> Delete_(std::shared_ptr<Node<Key>>  node, const Key& key) {
        if (node == nullptr) {
            return node;
        }
        if (node->key == key) {
            if (node->right == nullptr) {
                return node->left;
            }
            auto min = FindMin(node->right);
            node->right = UnlinkMin(node->right);
            min->left = node->left;
            min->right = node->right;
            return Balance(min);
        } else if(cmp(node->key, key)) {
            node->right = Balance(Delete_(node->right, key));
        } else {
            node->left = Balance(Delete_(node->left, key));
        }
        return Balance(node);
    }

    std::shared_ptr<Node<Key>> Insert_(std::shared_ptr<Node<Key>>  node, const Key& key) {
        if (node == nullptr) {
            std::shared_ptr<Node<Key>> pointer = {std::allocator_traits<Allocator>::allocate(alloc_, 1), [&](Node<Key>* ptr){
                    std::allocator_traits<Allocator>::destroy(alloc_, ptr);
                    std::allocator_traits<Allocator>::deallocate(alloc_, ptr, 1);
                }};
            std::allocator_traits<Allocator>::construct(alloc_, pointer.get(), key);
            return pointer;
        }
        if (cmp(node->key, key)) {
            node->right = Balance(Insert_(node->right, key));
        } else {
            node->left = Balance(Insert_(node->left, key));
        }
        return Balance(node);
    }

    std::shared_ptr<Node<Key>> RightRotate(std::shared_ptr<Node<Key>> shared) {
        auto left = shared->left;
        shared->left = std::move(left->right);
        left->right = shared;
        FixHeight(shared);
        FixHeight(left);
        return left;
    }

    std::shared_ptr<Node<Key>> LeftRotate(std::shared_ptr<Node<Key>> shared) {
        auto right = shared->right;
        shared->right = std::move(right->left);
        right->left = shared;
        FixHeight(shared);
        FixHeight(right);
        return right;
    }

    std::shared_ptr<Node<Key>> Balance(std::shared_ptr<Node<Key>> node) {
        auto shared = node;
        FixHeight(shared);
        if (BFactor(shared) == -2) {
            if (BFactor(shared->left) == 1) {
                shared->left = LeftRotate(shared->left);
            }
            return RightRotate(shared);
        } else if (BFactor(shared) == 2) {
            if (BFactor(shared->left) == -1) {
                shared->right = RightRotate(shared->right);
            }
            return LeftRotate(shared);
        }
        return shared;
    }

    bool Contains_(std::shared_ptr<Node<Key>>  node, Key key) {
        if (node == nullptr) {
            return false;
        }
        if (node->key == key) {
            return true;
        }
        if (cmp(node->key, key)) {
            return Contains_(node->right, key);
        } else {
            return Contains_(node->left, key);
        }
    }

    std::shared_ptr<Node<Key>> UnlinkMin(std::shared_ptr<Node<Key>> node) {
        if (node->left == nullptr) {
            return node->right;
        }

        node->left = UnlinkMin(node->left);
        return Balance(node);
    }

    std::shared_ptr<Node<Key>> FindMin(std::shared_ptr<Node<Key>> node) {
        if (node->left == nullptr) {
            return node;
        }
        return FindMin(node->left);
    }

    std::shared_ptr<Node<Key>> FindMax(std::shared_ptr<Node<Key>> node) {
        if (node->right == nullptr) {
            return node;
        }
        return FindMax(node->right);
    }

    std::shared_ptr<Node<Key>> FindRoot(std::shared_ptr<Node<Key>> node) {
        if(node == root_) {
            return nullptr;
        }
        std::shared_ptr<Node<Key>> possible_root = root_;
        while(possible_root != nullptr) {
            if(cmp(possible_root->key, node->key)) {
                if(possible_root->right->key == node->key) {
                    return possible_root;
                } else{
                    possible_root = possible_root->right;
                }
            } else {
                if(possible_root->left->key == node->key) {
                    return possible_root;
                } else {
                    possible_root = possible_root->left;
                }
            }
        }
        return nullptr;
    }

    int64_t Height_(std::shared_ptr<Node<Key>> node) {
        return node == nullptr ? 0 : node->height;
    }

    int64_t BFactor(std::shared_ptr<Node<Key>> node) {
        if(node == nullptr) {
            return 0;
        }
        return Height_(node->right) - Height_(node->left);
    }

    void FixHeight(std::shared_ptr<Node<Key>> node) {
        if(node == nullptr) {
            return;
        }
        node->height = std::max(Height_(node->right), Height_(node->left)) + 1;
    }

    Comp cmp;
    std::shared_ptr<Node<Key>> root_;
    Allocator alloc_;
};

int main() {

    AVLtree<size_t> avl;
    for (int i = 0; i < 5; ++i) {
        avl.Insert(i);
    }
    for (auto&& i : avl) {
        std::cout << i;
    }
    for (int i = 0; i < 2; ++i) {
        avl.Delete(i);
    }

    for (auto&& i : avl) {
        std::cout << i;
    }

    return 0;
}
