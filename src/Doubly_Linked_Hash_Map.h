//
// Created by urani on 10/14/2025.
//

#ifndef Doubly_Linked_Hash_Map_H
#define Doubly_Linked_Hash_Map_H
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>
#include "Funnel_Hash_Map.h"
//#include <unordered_map>

template <typename Key, typename Value>
class Doubly_Linked_Hash_Map{
  public:
    struct NodeProps {
      Key next = NULL_KEY;
      Key prev = NULL_KEY;
      Value value = Value{};
    };
  protected:
    static constexpr Key NULL_KEY = std::numeric_limits<Key>::max();
    size_t node_count;
    Funnel_Hash_Map<Key, NodeProps> umap;
    //std::unordered_map<Key, NodeProps> umap;
    Key head = NULL_KEY;
    Key tail = NULL_KEY;
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
        Doubly_Linked_Hash_Map* map_ptr;
        Key curr_key;
        iterator(Doubly_Linked_Hash_Map* map, Key key) : map_ptr(map), curr_key(key) {}

        // Allow Doubly_Linked_Hash_Map and const_iterator to access private members
        friend class Doubly_Linked_Hash_Map;
        friend class const_iterator;

    public:
        iterator() : map_ptr(nullptr), curr_key(NULL_KEY) {}

        reference operator*() const {
            return map_ptr->umap.find(curr_key)->second.value;
        }

        pointer operator->() const {
            return &(map_ptr->umap.find(curr_key)->second.value);
        }

        const Key& key() const {
            return curr_key;
        }

        Value& value() const {
            return map_ptr->umap.find(curr_key)->second.value;
        }

        // Pre-increment (++it)
        iterator& operator++() {
            curr_key = map_ptr->umap.find(curr_key)->second.next;
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
            if (curr_key == NULL_KEY) {
                curr_key = map_ptr->tail;
            } else {
                curr_key = map_ptr->umap.find(curr_key)->second.prev;
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
            return map_ptr == other.map_ptr && curr_key == other.curr_key;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

        // Mixed-const-nonconst comparison
        bool operator==(const const_iterator& other) const {
          return map_ptr == other.map_ptr && curr_key == other.curr_key;
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
        const Doubly_Linked_Hash_Map* map_ptr;
        Key curr_key;

        const_iterator(const Doubly_Linked_Hash_Map* map, Key key) : map_ptr(map), curr_key(key) {}

        // Allow Doubly_Linked_Hash_Map and iterator to access private members
        friend class Doubly_Linked_Hash_Map;
        friend class iterator;

    public:
        const_iterator() : map_ptr(nullptr), curr_key(NULL_KEY) {}

        // Converting constructor from iterator
        const_iterator(const iterator& other)
            : map_ptr(other.map_ptr), curr_key(other.curr_key) {}

        reference operator*() const {
            return map_ptr->umap.find(curr_key)->second.value;
        }

        pointer operator->() const {
            return &(map_ptr->umap.find(curr_key)->second.value);
        }

        const Key& key() const {
            return curr_key;
        }

        const Value& value() const {
            return map_ptr->umap.find(curr_key)->second.value;
        }

        // Pre-increment (++it)
        const_iterator& operator++() {
            curr_key = map_ptr->umap.find(curr_key)->second.next;
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
            if (curr_key == NULL_KEY) {
                curr_key = map_ptr->tail;
            } else {
                curr_key = map_ptr->umap.find(curr_key)->second.prev;
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
            return map_ptr == other.map_ptr && curr_key == other.curr_key;
        }

        bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }

        // Mixed-const-nonconst comparison
        bool operator==(const iterator& other) const {
            return map_ptr == other.map_ptr && curr_key == other.curr_key;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
    };

    explicit Doubly_Linked_Hash_Map(size_t N) : node_count(0),
      umap(N), head(NULL_KEY), tail(NULL_KEY)
    {}


    Doubly_Linked_Hash_Map(const Doubly_Linked_Hash_Map& other_Doubly_Linked_Hash_Map) :
      node_count(0), umap(other_Doubly_Linked_Hash_Map.umap),
      head(NULL_KEY), tail(NULL_KEY)
    {
      Key nav_node = other_Doubly_Linked_Hash_Map.getHead();
      for(size_t i = 0; i<other_Doubly_Linked_Hash_Map.nodeCount(); i++){
        this->addTail(nav_node,other_Doubly_Linked_Hash_Map.umap.find(nav_node)->second.value);
        nav_node = other_Doubly_Linked_Hash_Map.umap.find(nav_node)->second.next;
      }
    }

    Doubly_Linked_Hash_Map& operator=(const Doubly_Linked_Hash_Map& other_Doubly_Linked_Hash_Map){
      if(this != &other_Doubly_Linked_Hash_Map){
        clear(); //convert linked list to default
        Key nav_node = other_Doubly_Linked_Hash_Map.getHead();
        for(size_t i = 0; i<other_Doubly_Linked_Hash_Map.nodeCount(); i++){
          this->addTail(nav_node, other_Doubly_Linked_Hash_Map.umap.find(nav_node)->second.value);
          nav_node = other_Doubly_Linked_Hash_Map.umap.find(nav_node)->second.next;
        }
      }
      return *this;
    }

    ~Doubly_Linked_Hash_Map(){
      clear();
    }

    //Behaviors
    void printForward() const {
      Key nav_node = this->head;
      for(size_t i = 0; i<this->node_count; i++){
        std::cout << umap.find(nav_node)->second.value << std::endl;
        nav_node = umap.find(nav_node)->second.next;
      }
    }

    void printReverse() const {
      Key nav_node = this->tail;
      for(size_t i = 0; i<this->node_count; i++){
        std::cout << umap.find(nav_node)->second.value << std::endl;
        nav_node = umap.find(nav_node)->second.prev;
      }
    }

    //Accessors
    size_t nodeCount() const {
      return this->node_count;
    }

    iterator begin() {
        return iterator(this, this->head);
    }

    iterator end() {
        return iterator(this, NULL_KEY);
    }

    const_iterator begin() const {
        return const_iterator(this, this->head);
    }

    const_iterator end() const {
        return const_iterator(this, NULL_KEY);
    }

    const_iterator cbegin() const {
        return const_iterator(this, this->head);
    }

    const_iterator cend() const {
        return const_iterator(this, NULL_KEY);
    }

    iterator find(const Key& key) {
        //if (umap.contains(key)) {
        if(umap.find(key) != umap.end()){
            return iterator(this, key);
        }
        return end();
    }

    const_iterator find(const Key& key) const {
        if (umap.contains(key)) {
            return const_iterator(this, key);
        }
        return cend();
    }

    bool empty() const {
      if(node_count == 0) {
        return true;
      }
      return false;
    }

    bool contains(const Key& key) const {
        //return umap.contains(key);
        return umap.find(key) != umap.end();
    }

    std::vector<Key> findValues(const Value& value) {
      std::vector<Key> keys;
      Key nav_node = this->head;
      for(size_t i = 0; i<this->node_count; i++){
        if(umap.find(nav_node)->second.value == value){
          keys.push_back(nav_node);
        }
        nav_node = umap.find(nav_node)->second.next;
      }
      return keys;
    }

    std::vector<Key> findValues(const Value& value) const {
      std::vector<Key> keys;
      Key nav_node = this->head;
      for(size_t i = 0; i<this->node_count; i++){
        if(umap.find(nav_node)->second.value == value){
          keys.push_back(nav_node);
        }
        nav_node = umap.find(nav_node)->second.next;
      }
      return keys;
    }

    Key getNode(size_t index){
      if(index >= node_count){
        throw std::out_of_range("No node at index!");
      }
      Key nav_node = this->head;
      for(size_t i = 0; i<index; i++){
        nav_node = umap.find(nav_node)->second.next;
      }
      return nav_node;
    }

    Key getNode(const size_t index) const{
      if(index >= node_count){
        throw std::out_of_range("No node at index!");
      }
      Key nav_node = this->head;
      for(int i = 0; i<index; i++){
        nav_node = umap.find(nav_node)->second.next;
      }
      return nav_node;
    }

    Key getHead() const {
      return this->head;
    }

    Key getTail() const {
      return this->tail;
    }

    //Insertions
    void addHead(const Key& key, const Value& value){
      if (key == NULL_KEY) {
        throw std::invalid_argument("Key value is reserved and cannot be inserted.");
      }
      //if (umap.contains(key)) {
      if(umap.find(key) != umap.end()){
        return;
        //throw std::invalid_argument("Key already exists in the map.");
      }
      umap.emplace(key, NodeProps{NULL_KEY, NULL_KEY, value});
      Key new_head = key;
      if(node_count == 0){
        this->head = new_head;
        this->tail = new_head;
      }
      else {
        umap.find(this->head)->second.prev = new_head;
        umap.find(new_head)->second.next = this->head;
        this->head = new_head;
      }
      node_count++;
    }

    void addTail(const Key& key, const Value& value){
      if (key == NULL_KEY) {
        throw std::invalid_argument("Key value is reserved and cannot be inserted.");
      }
      //if (umap.contains(key)) {
      if(umap.find(key) != umap.end()){
        throw std::invalid_argument("Key already exists in the map.");
      }
      umap.emplace(key, NodeProps{NULL_KEY, NULL_KEY, value});

      Key new_tail = key;
      if(node_count == 0){
        this->head = new_tail;
        this->tail = new_tail;
      }
      else{
        umap.find(this->tail)->second.next = new_tail;
        umap.find(new_tail)->second.prev = this->tail;
        this->tail = new_tail;
      }
      node_count++;
    }

    void insertBefore(const Key& key, const Value& value, const Key& some_node){
      if (key == NULL_KEY) {
        throw std::invalid_argument("Key value is reserved and cannot be inserted.");
      }
      //if (umap.contains(key)) {
      if(umap.find(key) != umap.end()){
        throw std::invalid_argument("Key already exists in the map.");
      }
      //if (!umap.contains(some_node)) {
      if(umap.find(some_node) == umap.end()){
        throw std::out_of_range("Node to insert before does not exist.");
      }

      if(some_node == this->head){
        addHead(key,value);
        return;
      }

      Key prev_node_key = umap.find(some_node)->second.prev;

      // Create node and update output links
      umap.emplace(key, NodeProps{some_node, prev_node_key, value});
      Key node2insert = key;

      // Update input links
      umap.find(prev_node_key)->second.next = node2insert;
      umap.find(some_node)->second.prev = node2insert;

      node_count++;
    }

    void insertAfter(const Key& key, const Value& value, const Key& some_node){
      if (key == NULL_KEY) {
        throw std::invalid_argument("Key value is reserved and cannot be inserted.");
      }
      //if (umap.contains(key)) {
      if(umap.find(key) != umap.end()){
        throw std::invalid_argument("Key already exists in the map.");
      }
      //if (!umap.contains(some_node)) {
      if(umap.find(some_node) == umap.end()){
        throw std::out_of_range("Node to insert after does not exist.");
      }

      if(some_node == this->tail){
        addTail(key,value);
        return;
      }

      Key next_node_key = umap.find(some_node)->second.next;

      // Create node and update output links
      umap.emplace(key, NodeProps{next_node_key, some_node, value});
      Key node2insert = key;

      // Update input links
      umap.find(next_node_key)->second.prev = node2insert;
      umap.find(some_node)->second.next = node2insert;

      node_count++;
    }

    void insertAt(const Key& key, const Value& value, const size_t index){
      if (key == NULL_KEY) {
        throw std::invalid_argument("Key value is reserved and cannot be inserted.");
      }
      if (umap.contains(key)) {
        throw std::invalid_argument("Key already exists in the map.");
      }

      if(index > node_count){
        throw std::out_of_range("Index out of range!");
      }
      else if(index == 0){
        addHead(key,value);
        return;
      }
      else if(index == node_count){
        addTail(key,value);
        return;
      }

      Key some_node = getNode(index);
      insertBefore(key, value, some_node);
    }

    //Removals
    bool removeHead(){
      if(node_count == 0){
        return false;
      }
      Key old_head = this->head;
      if(node_count == 1){
        this->head = NULL_KEY;
        this->tail = NULL_KEY;
      }
      else{
        this->head = umap.find(old_head)->second.next;
        umap.find(this->head)->second.prev = NULL_KEY;
      }
      umap.erase(old_head);
      --this->node_count;
      return true;
    }

    bool removeTail(){
      if(node_count == 0){
        return false;
      }
      Key old_tail = this->tail;
      if(node_count == 1){
        this->head = NULL_KEY;
        this->tail = NULL_KEY;
      }
      else{
        this->tail = umap.find(old_tail)->second.prev;
        umap.find(this->tail)->second.next = NULL_KEY;
      }
      umap.erase(old_tail);
      --this->node_count;
      return true;
    }

    bool removeAt(const size_t index){
      if(index >= node_count) {
          return false;
      }
      if(index == 0){
        return removeHead();
      }
      else if(index == node_count-1){
        return removeTail();
      }

      Key nav_node = getNode(index); // Use getNode helper

      Key temp_next = umap.find(nav_node)->second.next;
      Key temp_prev = umap.find(nav_node)->second.prev;

      umap.find(temp_prev)->second.next = temp_next;
      umap.find(temp_next)->second.prev = temp_prev;
      umap.erase(nav_node);

      node_count--;
      return true;
    }

    bool remove(const Key& key){
      if(!umap.contains(key)){
        return false;
      }
      if(key == this->head){
        return removeHead();
      }
      else if(key == this->tail){
        return removeTail();
      }
      else{
        Key temp_next = umap.find(key)->second.next;
        Key temp_prev = umap.find(key)->second.prev;

        umap.find(temp_prev)->second.next = temp_next;
        umap.find(temp_next)->second.prev = temp_prev;
        umap.erase(key);

        node_count--;
      }
      return true;
    }

    int removeNodesWithValue(const Value& value){
      int removal_count = 0;
      Key nav_node = this->head;
      while(nav_node != NULL_KEY){
        Key backup = umap.find(nav_node)->second.next;
        if(umap.find(nav_node)->second.value == value){
          if(remove(nav_node)) {
            removal_count++;
          }
        }
        nav_node = backup;
      }
      return removal_count;
    }

    void clear(){
      umap.clear();
      this->head = NULL_KEY;
      this->tail = NULL_KEY;
      this->node_count = 0;
    }

    //Operators
    Value& operator[](const int index){
      if(index >= node_count || index < 0){
        throw std::out_of_range("No node at index!");
      }
      Key nav_node = this->head;
      for(int i = 0; i<index; i++){
        nav_node = umap.find(nav_node)->second.next;
      }
      return umap.find(nav_node)->second.value;
    }

    const Value& operator[](const int index) const {
      if(index >= node_count || index < 0){
        throw std::out_of_range("No node at index!");
      }
      Key nav_node = this->head;
      for(int i = 0; i<index; i++){
        nav_node = umap.find(nav_node)->second.next;
      }
      return umap.find(nav_node)->second.value;
    }


    bool operator==(const Doubly_Linked_Hash_Map& other_Doubly_Linked_Hash_Map) const {
      if(this->node_count != other_Doubly_Linked_Hash_Map.nodeCount()){
        return false;
      }
      Key nav_node = this->head;
      Key other_nav_node = other_Doubly_Linked_Hash_Map.getHead();
      for(size_t i = 0; i<node_count; i++){
        if(umap.find(nav_node)->second.value != other_Doubly_Linked_Hash_Map.umap.find(other_nav_node)->second.value){
          return false;
        }
        nav_node = umap.find(nav_node)->second.next;
        other_nav_node = other_Doubly_Linked_Hash_Map.umap.find(other_nav_node)->second.next;
      }
      return true;
    }

    bool operator!=(const Doubly_Linked_Hash_Map& other_Doubly_Linked_Hash_Map) const {
      return !(*this == other_Doubly_Linked_Hash_Map);
    }

};
#endif //Doubly_Linked_Hash_Map_H
