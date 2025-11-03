//
// Created by urani on 11/2/2025.
//

#ifndef AVL_TREE_H
#define AVL_TREE_H
//#include <iostream>
#include <stack>
#include <queue>
#include <limits>

template<typename Key, typename Value>
class AVL_Tree{
    public:
        struct Node{
            std::pair<Key,Value> data;
            Node* left;
            Node* right;
            size_t height;
            int balance_factor;
            Node() : left(nullptr), right(nullptr), height(0), balance_factor(0){}
        };
    private:
        Node* root;
        size_t node_count;

        void update_H_and_BF(Node* back_node) {
            int left_height = -1;
            int right_height = -1;
            if(back_node->left != nullptr) {
                left_height = back_node->left->height;
            }
            if(back_node->right != nullptr) {
                right_height = back_node->right->height;
            }
            back_node->height = 1 + std::max(left_height,right_height);
            back_node->balance_factor = left_height - right_height;
        }
        void balance_tree(std::stack<Node*>& node_storage, bool deletion = false) {
            while(!node_storage.empty()) {
                Node* nav_node = node_storage.top();
                node_storage.pop();
                update_H_and_BF(nav_node);
                Node* new_subtree_root = nav_node;

                //perform rotations if needed
                if(nav_node->balance_factor == -2) {
                    if(nav_node->right->balance_factor == -1 or nav_node->right->balance_factor == 0) {
                        new_subtree_root = rotateLeft(nav_node);
                    }
                    else if(nav_node->right->balance_factor == 1) {
                        new_subtree_root = rotateRightLeft(nav_node);
                    }
                }
                else if(nav_node->balance_factor == 2) {
                    if(nav_node->left->balance_factor == 1 or nav_node->left->balance_factor == 0) {
                        new_subtree_root = rotateRight(nav_node);
                    }
                    else if(nav_node->left->balance_factor == -1) {
                        new_subtree_root = rotateLeftRight(nav_node);
                    }
                }

                //connect the subtree back to the tree
                if(node_storage.empty()) {
                    this->root = new_subtree_root;
                    continue;
                }
                if(node_storage.top()->left == nav_node) {
                    node_storage.top()->left = new_subtree_root;
                }
                else {
                    node_storage.top()->right = new_subtree_root;
                }

                //check if done
                if (new_subtree_root != nav_node and deletion == false) {
                    break;
                }
            }
        }

        Node* rotateLeft(Node* grandparent){
            /*
                g        to        p
                 \                / \
                  p              g   c
                   \
                    c
            */
            Node* parent = grandparent->right;
            Node* temp_left = parent->left;
            parent->left = grandparent;
            grandparent->right = temp_left;
            update_H_and_BF(grandparent);
            update_H_and_BF(parent);
            return parent;
        }

        Node* rotateRight(Node* grandparent){
            /*
                     g    to        p
                    /              / \
                   p              c   g
                  /
                 c
            */
            Node* parent = grandparent->left;
            Node* temp_right = parent->right;
            parent->right = grandparent;
            grandparent->left = temp_right;
            update_H_and_BF(grandparent);
            update_H_and_BF(parent);
            return parent;
        }

        Node* rotateRightLeft(Node* grandparent){
            /*
                    g     to        c
                     \             / \
                      p           g   p
                     /
                    c
            */
            //get parent, child, and child's left and right
            Node* parent = grandparent->right;
            Node* child = grandparent->right->left;
            Node* temp_left = child -> left;
            Node* temp_right = child -> right;
            //create cycles
            child -> left = grandparent;
            child -> right = parent;
            //break cycles
            parent -> left = temp_right;
            grandparent -> right = temp_left;
            update_H_and_BF(grandparent);
            update_H_and_BF(parent);
            update_H_and_BF(child);
            return child;
        }

        Node* rotateLeftRight(Node* grandparent){
            /*
                g         to        c
               /                   / \
              p                   p   g
               \
                c
            */
            //get parent, child, and child's left and right
            Node* parent = grandparent->left;
            Node* child = grandparent->left->right;
            Node* temp_left = child -> left;
            Node* temp_right = child -> right;
            //create cycles
            child -> right = grandparent;
            child -> left = parent;
            //break cycles
            parent -> right = temp_left;
            grandparent -> left = temp_right;
            update_H_and_BF(grandparent);
            update_H_and_BF(parent);
            update_H_and_BF(child);
            return child;
        }

        Node* successor(Node* nav_node){
            // Case 1: If node has a right subtree,
            // the successor is the leftmost node in the right subtree.
            if(nav_node != nullptr && nav_node->right != nullptr){
                Node* succ = nav_node->right;
                while(succ->left != nullptr){
                    succ = succ->left;
                }
                return succ;
            }

            // Case 2: If node has no right subtree,
            // the successor is the lowest ancestor for which
            // the navigation node is in its left subtree.
            // We go back to the root instead of continuing
            // at the navigation node.
            Node* succ = nullptr;
            Node* nav_node2 = this->root;
            while (nav_node2 != nullptr) {
                if (nav_node->data.first < nav_node2->data.first) {
                    succ = nav_node2;
                    nav_node2 = nav_node2->left;
                }
                else if (nav_node->data.first > nav_node2->data.first) {
                    nav_node2 = nav_node2->right;
                }
                else {
                    // Arrived at the original node
                    break;
                }
            }
            return succ;
        }

        Node* predecessor(Node* nav_node){
            // Case 1: If node has a left subtree,
            // the successor is the rightmost node in the left subtree.
            if(nav_node != nullptr && nav_node->left != nullptr){
                Node* pred = nav_node->left;
                while(pred->right != nullptr){
                    pred = pred->right;
                }
                return pred;
            }

            // Case 2: If node has no left subtree,
            // the predecessor is the lowest ancestor for which
            // the navigation node is in its right subtree.
            // We go back to the root instead of continuing
            // at the navigation node.
            Node* pred = nullptr;
            Node* nav_node2 = this->root;
            while (nav_node2 != nullptr) {
                if (nav_node->data.first > nav_node2->data.first) {
                    pred = nav_node2;
                    nav_node2 = nav_node2->right;
                }
                else if (nav_node->data.first < nav_node2->data.first) {
                    nav_node2 = nav_node2->left;
                }
                else {
                    // Arrived at the original node
                    break;
                }
            }
            return pred;
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
            AVL_Tree* AVL_Tree_ptr;
            Node* node_ptr;

            iterator(AVL_Tree* AVL_Tree, Node* node) : AVL_Tree_ptr(AVL_Tree), node_ptr(node) {}

            // Allow AVL_Tree and const_iterator to access private members
            friend class AVL_Tree;
            friend class const_iterator;

        public:
            iterator() : AVL_Tree_ptr(nullptr), node_ptr(nullptr) {}

            reference operator*() const {
                return node_ptr->data.second;
            }

            pointer operator->() const {
                return &(node_ptr->data.second);
            }

            const Key& key() const {
                return node_ptr->data.first;
            }

            Value& value() const {
                return node_ptr->data.second;
            }

            // Pre-increment (++it)
            iterator& operator++() {
                node_ptr = AVL_Tree_ptr->successor(node_ptr);
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
                    Node* max_node = AVL_Tree_ptr->root;
                    if (max_node != nullptr) {
                        while (max_node->right != nullptr) max_node = max_node->right;
                    }
                    node_ptr = max_node;
                } else {
                    node_ptr = AVL_Tree_ptr->predecessor(node_ptr);
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
                return AVL_Tree_ptr == other.AVL_Tree_ptr && node_ptr == other.node_ptr;
            }

            bool operator!=(const iterator& other) const {
                return !(*this == other);
            }

            // Mixed-const-nonconst comparison
            bool operator==(const const_iterator& other) const {
                return AVL_Tree_ptr == other.AVL_Tree_ptr && node_ptr == other.node_ptr;
            }
            bool operator!=(const const_iterator& other) const {
                return !(*this == other);
            }
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
            const AVL_Tree* AVL_Tree_ptr;
            Node* node_ptr;

            const_iterator(const AVL_Tree* map, Node* node) : AVL_Tree_ptr(map), node_ptr(node) {}

            friend class AVL_Tree;
            friend class iterator;

        public:
            const_iterator() : AVL_Tree_ptr(nullptr), node_ptr(nullptr) {}

            // Converting constructor from non-const iterator
            const_iterator(const iterator& other)
                : AVL_Tree_ptr(other.AVL_Tree_ptr), node_ptr(other.node_ptr) {}

            reference operator*() const {
                return node_ptr->data.second;
            }

            pointer operator->() const {
                return &(node_ptr->data.second);
            }

            const Key& key() const {
                return node_ptr->data.first;
            }

            const Value& value() const {
                return node_ptr->data.second;
            }

            // Pre-increment (++it)
            const_iterator& operator++() {
                node_ptr = AVL_Tree_ptr->successor(node_ptr);
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
                    Node* max_node = AVL_Tree_ptr->root;
                    if (max_node != nullptr) {
                        while (max_node->right != nullptr) max_node = max_node->right;
                    }
                    node_ptr = max_node;
                } else {
                    node_ptr = AVL_Tree_ptr->predecessor(node_ptr);
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
                return AVL_Tree_ptr == other.AVL_Tree_ptr && node_ptr == other.node_ptr;
            }

            bool operator!=(const const_iterator& other) const {
                return !(*this == other);
            }

            // Mixed-const-nonconst comparison
            bool operator==(const iterator& other) const {
                return AVL_Tree_ptr == other.AVL_Tree_ptr && node_ptr == other.node_ptr;
            }

            bool operator!=(const iterator& other) const {
                return !(*this == other);
            }
        };
    
        AVL_Tree() : root(nullptr), node_count(0)
        {}

        ~AVL_Tree(){
            this->clear();
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

        size_t size() const noexcept {
            return this->node_count;
        }

        size_t max_size() const noexcept {
            return std::numeric_limits<std::ptrdiff_t>::max();
        }

        size_t count(const Key& key) const{
            if(this->find(key) != this->end()){
                return 1;
            }
            return 0;
        }

        bool insert(const std::pair<Key, Value>& map_pair){
            return insert(map_pair.first,map_pair.second);
        }

        bool insert(const Key& key, const Value& value){
            Node* new_node = new Node();
            new_node->data.first = key;
            new_node->data.second = value;
            std::stack<Node*> node_storage;
            if(this->root == nullptr){
                this->root = new_node;
                this->root->height = 0;
                ++this->node_count;
                return true;
            }
            Node* nav_node = this->root;
            Node* follower_node = nullptr;
            while(true){
                node_storage.push(nav_node);
                if(key > nav_node->data.first){
                    follower_node = nav_node;
                    nav_node = nav_node->right;
                    if(nav_node == nullptr){
                        follower_node->right = new_node;
                        break;
                    }
                }
                else if(key < nav_node->data.first){
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
            balance_tree(node_storage);
            ++this->node_count;
            return true;
        }

        bool erase(const Key& key){
            if(this->root == nullptr) {
                return false;
            }

            std::stack<Node*> node_storage;

            // Root case
            if(this->root->data.first == key){
                if (this->root->left == nullptr && this->root->right == nullptr) { // Leaf node
                    delete this->root;
                    this->root = nullptr;
                }
                else if(this->root->left != nullptr && this->root->right != nullptr){ // Has two children
                    node_storage.push(this->root);
                    Node* successor = this->root->right;
                    while (successor->left != nullptr) {
                        node_storage.push(successor);
                        successor = successor->left;
                    }
                    this->root->data = successor->data;
                    if(node_storage.top() == this->root) {
                        node_storage.top()->right = successor->right;
                    }
                    else {
                        node_storage.top()->left = successor->right;
                    }
                    delete successor;
                    balance_tree(node_storage, true);
                }
                else { // Has one child
                    Node* old_root = this->root;

                    if(this->root->left != nullptr) {
                        this->root = this->root->left;
                    } else {
                        this->root = this->root->right;
                    }

                    delete old_root;
                }
                --this->node_count;
                return true; // <-- BUG FIX
            }

            // Non-root case
            Node* nav_node = this->root;
            while(nav_node != nullptr) {
                node_storage.push(nav_node);
                if(key < nav_node->data.first) {
                    nav_node = nav_node->left;
                }
                else if(key > nav_node->data.first){
                    nav_node = nav_node->right;
                }
                else { // Found the node to delete
                    node_storage.pop();

                    if(nav_node->left == nullptr && nav_node->right == nullptr) { // Leaf node
                        if(node_storage.top()->left == nav_node) {
                            node_storage.top()->left = nullptr;
                        }
                        else {
                            node_storage.top()->right = nullptr;
                        }
                        delete nav_node;
                    }
                    else if(nav_node->left != nullptr && nav_node->right != nullptr){ // Has two children
                        node_storage.push(nav_node);
                        Node* successor_parent = nav_node;
                        Node* successor = nav_node->right;
                        while (successor->left != nullptr) {
                            successor_parent = successor;
                            node_storage.push(successor);
                            successor = successor->left;
                        }
                        nav_node->data = successor->data;

                        // Unlink successor
                        if(successor_parent == nav_node) {
                            successor_parent->right = successor->right;
                        }
                        else {
                            successor_parent->left = successor->right;
                        }
                        delete successor;
                    }
                    else { // Has one child

                        Node* child;
                        if(nav_node->left != nullptr) {
                            child = nav_node->left;
                        } else {
                            child = nav_node->right;
                        }

                        if(node_storage.top()->left == nav_node) {
                            node_storage.top()->left = child;
                        }
                        else {
                            node_storage.top()->right = child;
                        }
                        delete nav_node;
                    }

                    balance_tree(node_storage, true);
                    --this->node_count;
                    return true;
                }
            }
        return false; // Key not found
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

            return iterator(this, potential_pred);
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

            return iterator(this, potential_succ);
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

            return const_iterator(this, potential_pred);
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

            return const_iterator(this, potential_succ);
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

    /*
        void generate_debug_visual(const string& file_name) {
            ofstream visual_file(file_name, ios_base::out);
            try {
                if(!visual_file.is_open()) {
                    throw runtime_error("Could not create visual file!");
                }
                visual_file << "digraph AVLTree {" << endl;
                visual_file << "     node [fontname=\"Arial\", shape=circle, style=filled, fillcolor=\"#00FFF7\"];" << endl;
                visual_file << "     edge [arrowhead=vee];" << endl;

                if (this->root == nullptr) {
                    visual_file << "  empty [label=\"AVLTree is empty\", shape=plaintext];" << endl;
                }
                else {
                    Node* nav_node = this->root;
                    start_debug_graph_generation(nav_node, visual_file);
                }
                visual_file << "}" << endl;
            }
            catch(exception& e){
                cerr << e.what() << endl;
            }
        }


        void start_debug_graph_generation(Node* nav_node, ofstream& visual_file) {
            if (nav_node != nullptr) {
                string parent_ID_as_string = getIDAsString(nav_node);
                visual_file << "  \"" << parent_ID_as_string << "\";" << endl;
                if(nav_node->left != nullptr) {
                    visual_file << "  \"" << parent_ID_as_string << "\" -> \"" << getIDAsString(nav_node->left) << "\";" << endl;
                    start_debug_graph_generation(nav_node->left, visual_file);
                }
                if(nav_node->right != nullptr){
                    visual_file << "  \"" << parent_ID_as_string << "\" -> \"" << getIDAsString(nav_node->right) << "\";" << endl;
                    start_debug_graph_generation(nav_node->right, visual_file);
                }
            }
        }

        */
};
#endif //AVL_TREE_H
