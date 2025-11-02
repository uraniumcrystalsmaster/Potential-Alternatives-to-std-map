//
// Created by urani on 11/1/2025.
//

#ifndef TREAP_H
#define TREAP_H
#include <chrono>
#include <random>
#include <vector>
#include <queue>
template<typename Key, typename Value>
class Treap{
    public:
        struct Node{
            std::pair<Key,Value> data;
            Node* left;
            Node* right;
            size_t priority;
            Node() : left(nullptr), right(nullptr){}
        };
    private:
        Node* root;
        size_t node_count;
        std::mt19937_64 engine;
        std::uniform_int_distribution<std::size_t> dist_size_t;

        Node* successor(const Key key){
            Node* nav_node = find(key);
            if(nav_node == nullptr){
                return this->end();
            }
            if(nav_node->right){
                nav_node = nav_node->right;
                while(nav_node->left != nullptr){
                    nav_node = nav_node->left;
                }
                return nav_node;
            }
            Node* succ = nullptr;
            Node* nav_node2 = this->root;
            while (nav_node2 != nullptr) {
                // Could be successor
                if (nav_node->data.first < nav_node2->key) {
                    succ = nav_node2;
                    nav_node2 = nav_node2->left;
                }
                // Cannot be successor
                else if (nav_node->data.first > nav_node2->key) {
                    nav_node2 = nav_node2->right;
                }
                else {
                    // Arrived at the original node,
                    // which means we found the successsor
                    break;
                }
            }
            return succ;
        }

        Node* predecessor(const Key key){
            Node* nav_node = find(key);
            if(nav_node == nullptr){
                return this->end();
            }
            if(nav_node->left){
                nav_node = nav_node->left;
                while(nav_node->right != nullptr){
                    nav_node = nav_node->right;
                }
                return nav_node;
            }
            Node* pred = nullptr;
            Node* nav_node2 = this->root;
            while (nav_node2 != nullptr) {
                // Could be successor
                if (nav_node->data.first > nav_node2->key) {
                    pred = nav_node2;
                    nav_node2 = nav_node2->right;
                }
                // Cannot be successor
                else if (nav_node->data.first < nav_node2->key) {
                    nav_node2 = nav_node2->left;
                }
                else {
                    // Arrived at the original node,
                    // which means we found the successsor
                    break;
                }
            }
            return pred;
        }

        void bubble_up(std::vector<Node*>& path2parent, Node* inserted_node) {

            Node* child = inserted_node;

            for (size_t i = 0; i < path2parent.size(); i++) {
                Node* parent = path2parent[path2parent.size()-i-1];

                if (child->priority < parent->priority) {
                    Node* new_subtree_root = nullptr;

                    if (parent->right == child) {
                        new_subtree_root = rotateLeft(parent);
                    }
                    else {
                        new_subtree_root = rotateRight(parent);
                    }

                    if (i == path2parent.size()-1) {
                        this->root = new_subtree_root;
                    }
                    else {
                        Node* grandparent = path2parent[path2parent.size()-i-2];

                        if (grandparent->left == parent) {
                            grandparent->left = new_subtree_root;
                        }
                        else {
                            grandparent->right = new_subtree_root;
                        }
                    }
                    child = new_subtree_root;

                }
                else {
                    break; //Yay! We're done!
                }
            }
        }
    public:
        class const_iterator;

        class iterator {
        public:
            // Iterator traits
            using iterator_category = std::bidirectional_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = Value;
            using pointer           = Value*;
            using reference         = Value&;

        private:
            Treap* treap_ptr;
            Node* node_ptr;

            iterator(Treap* treap, Node* node) : treap_ptr(treap), node_ptr(node) {}

            // Allow Treap and const_iterator to access private members
            friend class Treap;
            friend class const_iterator;

        public:
            iterator() : treap_ptr(nullptr), node_ptr(nullptr) {}

            reference operator*() const {
                return node_ptr->value;
            }

            pointer operator->() const {
                return &(node_ptr->value);
            }

            const Key& key() const {
                return node_ptr->key;
            }

            Value& value() const {
                return node_ptr->value;
            }

            // Pre-increment (++it)
            iterator& operator++() {
                node_ptr = treap_ptr->successor(node_ptr->key);
                return *this;
            }

            // Post-increment (it++)
            iterator operator++(int) {
                iterator old = *this; // Make a copy
                ++(*this);           // Increment self
                return old;          // Return the old copy
            }

            // Pre-decrement (--it)
            iterator& operator--() {
                if (node_ptr == nullptr) {
                    Node* max_node = treap_ptr->root;
                    if (max_node != nullptr) {
                        while (max_node->right != nullptr) max_node = max_node->right;
                    }
                    node_ptr = max_node;
                } else {
                    node_ptr = treap_ptr->predecessor(node_ptr->key);
                }
                return *this;
            }

            // Post-decrement (it--)
            iterator operator--(int) {
                iterator old = *this; // Make a copy
                --(*this);           // Decrement self
                return old;          // Return the old copy
            }

            // Comparison operators
            bool operator==(const iterator& other) const {
                return treap_ptr == other.treap_ptr && node_ptr == other.node_ptr;
            }

            bool operator!=(const iterator& other) const {
                return !(*this == other);
            }

            // Mixed-const-nonconst comparison
            bool operator==(const const_iterator& other) const;
            bool operator!=(const const_iterator& other) const;
        };

        class const_iterator {
        public:
            // Iterator traits
            using iterator_category = std::bidirectional_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = const Value;
            using pointer           = const Value*;
            using reference         = const Value&;

        private:
            const Treap* treap_ptr;
            Node* node_ptr;

            const_iterator(const Treap* map, Node* node) : treap_ptr(map), node_ptr(node) {}

            friend class Treap;
            friend class iterator;

        public:
            const_iterator() : treap_ptr(nullptr), node_ptr(nullptr) {}

            // Converting constructor from non-const iterator
            const_iterator(const iterator& other)
                : treap_ptr(other.treap_ptr), node_ptr(other.node_ptr) {}

            reference operator*() const {
                return node_ptr->value;
            }

            pointer operator->() const {
                return &(node_ptr->value);
            }

            const Key& key() const {
                return node_ptr->key;
            }

            const Value& value() const {
                return node_ptr->value;
            }

            // Pre-increment (++it)
            const_iterator& operator++() {
                node_ptr = treap_ptr->successor(node_ptr->key);
                return *this;
            }

            // Post-increment (it++)
            const_iterator operator++(int) {
                const_iterator old = *this; // Make a copy
                ++(*this);                 // Increment self
                return old;                // Return the old copy
            }

            // Pre-decrement (--it)
            const_iterator& operator--() {
                if (node_ptr == nullptr) {
                    Node* max_node = treap_ptr->root;
                    if (max_node != nullptr) {
                        while (max_node->right != nullptr) max_node = max_node->right;
                    }
                    node_ptr = max_node;
                } else {
                    node_ptr = treap_ptr->predecessor(node_ptr->key);
                }
                return *this;
            }

            // Post-decrement (it--)
            const_iterator operator--(int) {
                const_iterator old = *this; // Make a copy
                --(*this);                 // Decrement self
                return old;                // Return the old copy
            }

            // Comparison operators
            bool operator==(const const_iterator& other) const {
                return treap_ptr == other.treap_ptr && node_ptr == other.node_ptr;
            }

            bool operator!=(const const_iterator& other) const {
                return !(*this == other);
            }

            // Mixed-const-nonconst comparison
            bool operator==(const iterator& other) const {
                return treap_ptr == other.treap_ptr && node_ptr == other.node_ptr;
            }

            bool operator!=(const iterator& other) const {
                return !(*this == other);
            }
        };

    //init random ID generator and treap
    Treap() : root(nullptr),
          node_count(0),
          engine(std::random_device{}()),
          dist_size_t(0, std::numeric_limits<std::size_t>::max())
    {}

    ~Treap(){
        clear();
    }

    size_t size() const noexcept {
        return this->node_count;
    }

    size_t max_size() const noexcept {
        return std::numeric_limits<std::ptrdiff_t>::max();
    }

    template<class K>
    size_t count(const K& x) const{
        if(this->find(x) != this->end()){
            return 1;
        }
        return 0;
    }

    Node* rotateLeft(Node* node) {
        Node* new_root_of_subtree = node->right;
        node->right = new_root_of_subtree->left;
        new_root_of_subtree->left = node;
        return new_root_of_subtree;
    }

    Node* rotateRight(Node* node) {
        Node* new_root_of_subtree = node->left;
        node->left = new_root_of_subtree->right;
        new_root_of_subtree->right = node;
        return new_root_of_subtree;
    }

    bool insert(const std::pair<Key, Value>& map_pair){
        return insert(map_pair.first,map_pair.second);
    }

    bool insert(const Key& key, const Value& value){
        Node* new_node = new Node();
        new_node->data.first = key;
        new_node->data.second = value;
        new_node->priority = dist_size_t(engine);
        std::vector<Node*> path2parent;
        if(this->root == nullptr){
            this->root = new_node;
            this->node_count++;
            return true;
        }
        Node* nav_node = this->root;
        Node* follower_node = nullptr;
        while(true){
            path2parent.push_back(nav_node);
            if(new_node->data.first > nav_node->data.first){
                follower_node = nav_node;
                nav_node = nav_node->right;
                if(nav_node == nullptr){
                    follower_node->right = new_node;
                    break;
                }
            }
            else if(new_node->data.first < nav_node->data.first){
                follower_node = nav_node;
                nav_node = nav_node->left;
                if(nav_node == nullptr){
                    follower_node->left = new_node;
                    break;
                }
            }
            else{
                delete new_node;
                return false;
            }
        }
        bubble_up(path2parent, new_node);
        ++this->node_count;
        return true;
    }

    iterator find(const Key& key){
        Node* nav_node = this->root;
        while(nav_node != nullptr){
            if(key > nav_node->data.first){
                nav_node = nav_node->right;
            }
            else if(key < nav_node->data.first){
                nav_node = nav_node->left;
            }
            else{
                return iterator(this, nav_node);
            }
        }
        return iterator(this, nullptr);
    }

    const_iterator find(const Key& key) const {
        Node* nav_node = this->root;
        while(nav_node != nullptr){
            if(key > nav_node->data.first){
                nav_node = nav_node->right;
            }
            else if(key < nav_node->data.first){
                nav_node = nav_node->left;
            }
            else{
                return const_iterator(this, nav_node);
            }
        }
        return const_iterator(this, nullptr);
    }

    bool erase(const Key& key) {
        // 1. Find node to delete and its parent (follower node)
        Node* nav_node = this->root;
        Node* follower_node = nullptr;
        while(nav_node != nullptr){
            if(key > nav_node->data.first){
                follower_node = nav_node;
                nav_node = nav_node->right;
            }
            else if(key < nav_node->data.first){
                follower_node = nav_node;
                nav_node = nav_node->left;
            }
            else{
                break; // Found the node
            }
        }

        if (nav_node == nullptr) {
            return false;
        }

        // 2. Bubble down loop
        while (nav_node->left != nullptr or nav_node->right != nullptr) {
            Node* new_subtree_root = nullptr;

            // Promote the child
            if (nav_node->left == nullptr) {
                new_subtree_root = rotateLeft(nav_node);
            }
            else if (nav_node->right == nullptr) {
                new_subtree_root = rotateRight(nav_node);
            }
            else {
                if (nav_node->left->priority < nav_node->right->priority) {
                    new_subtree_root = rotateRight(nav_node);
                }
                else {
                    new_subtree_root = rotateLeft(nav_node);
                }
            }

            // Relink the parent to the new root
            if (follower_node == nullptr) {
                this->root = new_subtree_root;
            }
            else if (follower_node->left == nav_node) {
                follower_node->left = new_subtree_root;
            }
            else {
                follower_node->right = new_subtree_root;
            }

            follower_node = new_subtree_root;
        }

        // 3. Snip the leaf
        if (follower_node == nullptr) {
            this->root = nullptr;
        }
        else if (follower_node->left == nav_node) {
            follower_node->left = nullptr;
        }
        else {
            follower_node->right = nullptr;
        }

        delete nav_node;
        this->node_count--;
        return true;
    }

    void clear() {
        if (this->root == nullptr) {
            return;
        }
        std::queue<Node*> q;
        q.push(this->root);
        while(!q.empty()){
            Node* nav_node = q.front();
            q.pop();
            if(nav_node->left != nullptr){
                q.push(nav_node->left);
            }
            if(nav_node->right != nullptr){
                q.push(nav_node->right);
            }
            delete nav_node;
        }
        this->root = nullptr;
        this->node_count = 0;
    }

    iterator lower_bound(const Key& key) {
        Node* nav_node = this->root;
        Node* potential_pred = nullptr;

        while (nav_node != nullptr) {
            if (key < nav_node->data.first) {
                potential_pred = nav_node;
                nav_node = nav_node->left;
            }
            else if (key > nav_node->data.first) {
                nav_node = nav_node->right;
            }
            else {
                return iterator(this, nav_node);
            }
        }

        return iterator(this, nav_node);
    }

    iterator upper_bound(const Key& key) {
        Node* nav_node = this->root;
        Node* potential_succ = nullptr;

        while (nav_node != nullptr) {
            if (key < nav_node->data.first) {
                potential_succ = nav_node;
                nav_node = nav_node->left;
            }
            else {
                nav_node = nav_node->right;
            }
        }

        return iterator(this, nav_node);
    }

    const_iterator lower_bound(const Key& key) const {
        Node* nav_node = this->root;
        Node* potential_pred = nullptr;

        while (nav_node != nullptr) {
            if (key < nav_node->data.first) {
                potential_pred = nav_node;
                nav_node = nav_node->left;
            }
            else if (key > nav_node->data.first) {
                nav_node = nav_node->right;
            }
            else {
                return const_iterator(this, nav_node);
            }
        }

        return const_iterator(this, nav_node);
    }

    const_iterator upper_bound(const Key& key) const {
        Node* nav_node = this->root;
        Node* potential_succ = nullptr;

        while (nav_node != nullptr) {
            if (key < nav_node->data.first) {
                potential_succ = nav_node;
                nav_node = nav_node->left;
            }
            else {
                nav_node = nav_node->right;
            }
        }

        return const_iterator(this, nav_node);
    }

    iterator begin() {
        // Find the minimum node (go left as far as possible)
        Node* min_node = root;
        if (min_node != nullptr) {
            while (min_node->left != nullptr) {
                min_node = min_node->left;
            }
        }
        return iterator(this, min_node);
    }

    iterator end() {
        // 'end' is represented by a null node_ptr
        return iterator(this, nullptr);
    }

    const_iterator begin() const {
        Node* min_node = root;
        if (min_node != nullptr) {
            while (min_node->left != nullptr) {
                min_node = min_node->left;
            }
        }
        return const_iterator(this, min_node);
    }

    const_iterator end() const {
        return const_iterator(this, nullptr);
    }


};
#endif //TREAP_H
