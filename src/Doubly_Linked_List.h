//
// Created by urani on 11/21/2025.
//

#ifndef DOUBLY_LINKED_LIST_H
#define DOUBLY_LINKED_LIST_H
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>

template <typename T>
class Doubly_Linked_List {
  private:
    struct Node {
      T value;
      Node* next = nullptr;
      Node* prev = nullptr;

      Node(const T& val) : value(val) {}
    };

    Node* head = nullptr;
    Node* tail = nullptr;
    size_t node_count = 0;

  public:
    class const_iterator;

    class iterator {
    public:
      using iterator_category = std::bidirectional_iterator_tag;
      using difference_type   = std::ptrdiff_t;
      using value_type        = T;
      using pointer           = T*;
      using reference         = T&;

    private:
      Node* current;

      friend class Doubly_Linked_List;
      friend class const_iterator;

      iterator(Node* node) : current(node) {}

    public:
      iterator() : current(nullptr) {}

      reference operator*() const {
        return current->value;
      }

      pointer operator->() const {
        return &current->value;
      }

      iterator& operator++() {
        if (current) current = current->next;
        return *this;
      }

      iterator operator++(int) {
        iterator old = *this;
        ++(*this);
        return old;
      }

      iterator& operator--() {
        if (current) current = current->prev;
        return *this;
      }

      iterator operator--(int) {
        iterator old = *this;
        --(*this);
        return old;
      }

      bool operator==(const iterator& other) const {
        return current == other.current;
      }

      bool operator!=(const iterator& other) const {
        return !(*this == other);
      }
    };

    class const_iterator {
    public:
      using iterator_category = std::bidirectional_iterator_tag;
      using difference_type   = std::ptrdiff_t;
      using value_type        = const T;
      using pointer           = const T*;
      using reference         = const T&;

    private:
      const Node* current;

      friend class Doubly_Linked_List;

      const_iterator(const Node* node) : current(node) {}

    public:
      const_iterator() : current(nullptr) {}

      const_iterator(const iterator& other) : current(other.current) {}

      reference operator*() const {
        return current->value;
      }

      pointer operator->() const {
        return &current->value;
      }

      const_iterator& operator++() {
        if (current) current = current->next;
        return *this;
      }

      const_iterator operator++(int) {
        const_iterator old = *this;
        ++(*this);
        return old;
      }

      const_iterator& operator--() {
        if (current) current = current->prev;
        return *this;
      }

      const_iterator operator--(int) {
        const_iterator old = *this;
        --(*this);
        return old;
      }

      bool operator==(const const_iterator& other) const {
        return current == other.current;
      }

      bool operator!=(const const_iterator& other) const {
        return !(*this == other);
      }
    };

    template <typename L, typename G>
    friend void radix_sort(L& list, G get_value);

    Doubly_Linked_List() = default;

    Doubly_Linked_List(const Doubly_Linked_List& other) {
      for (const auto& val : other) {
        addTail(val);
      }
    }

    Doubly_Linked_List& operator=(const Doubly_Linked_List& other) {
      if (this != &other) {
        clear();
        for (const auto& val : other) {
          addTail(val);
        }
      }
      return *this;
    }

    ~Doubly_Linked_List() {
      clear();
    }

    void printForward() const {
      Node* temp = head;
      while (temp) {
        std::cout << temp->value << std::endl;
        temp = temp->next;
      }
    }

    void printReverse() const {
      Node* temp = tail;
      while (temp) {
        std::cout << temp->value << std::endl;
        temp = temp->prev;
      }
    }

    size_t size() const { return node_count; }

    bool empty() const { return node_count == 0; }

    iterator begin() { return iterator(head); }
    iterator end() { return iterator(nullptr); }
    const_iterator begin() const { return const_iterator(head); }
    const_iterator end() const { return const_iterator(nullptr); }
    const_iterator cbegin() const { return const_iterator(head); }
    const_iterator cend() const { return const_iterator(nullptr); }

    iterator find(const T& value) {
      Node* temp = head;
      while (temp) {
        if (temp->value == value) {
          return iterator(temp);
        }
        temp = temp->next;
      }
      return end();
    }

    const_iterator find(const T& value) const {
      Node* temp = head;
      while (temp) {
        if (temp->value == value) {
          return const_iterator(temp);
        }
        temp = temp->next;
      }
      return cend();
    }

    void addHead(const T& value) {
      Node* newNode = new Node(value);
      if (!head) {
        head = tail = newNode;
      }
      else {
        newNode->next = head;
        head->prev = newNode;
        head = newNode;
      }
      node_count++;
    }

    void addTail(const T& value) {
      Node* newNode = new Node(value);
      if (!tail) {
        head = tail = newNode;
      }
      else {
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
      }
      node_count++;
    }

    void insertAt(const T& value, size_t index) {
      if (index > node_count) {
        throw std::out_of_range("Index out of range!");
      }
      if (index == 0) {
        addHead(value);
        return;
      }
      if (index == node_count) {
        addTail(value);
        return;
      }

      Node* current = head;
      for (size_t i = 0; i < index; ++i) {
        current = current->next;
      }

      Node* newNode = new Node(value);
      newNode->prev = current->prev;
      newNode->next = current;
      current->prev->next = newNode;
      current->prev = newNode;
      node_count++;
    }

    bool removeHead() {
      if (!head) return false;
      Node* temp = head;
      head = head->next;
      if (head) {
        head->prev = nullptr;
      } else {
        tail = nullptr;
      }
      delete temp;
      node_count--;
      return true;
    }

    bool removeTail() {
      if (!tail) return false;
      Node* temp = tail;
      tail = tail->prev;
      if (tail) {
        tail->next = nullptr;
      } else {
        head = nullptr;
      }
      delete temp;
      node_count--;
      return true;
    }

    bool removeAt(size_t index) {
      if (index >= node_count) return false;
      if (index == 0) return removeHead();
      if (index == node_count - 1) return removeTail();

      Node* current = head;
      for (size_t i = 0; i < index; ++i) {
        current = current->next;
      }

      current->prev->next = current->next;
      current->next->prev = current->prev;
      delete current;
      node_count--;
      return true;
    }

    bool remove(const T& value) {
      Node* temp = head;
      while (temp) {
        if (temp->value == value) {
          if (temp == head) return removeHead();
          if (temp == tail) return removeTail();

          temp->prev->next = temp->next;
          temp->next->prev = temp->prev;
          delete temp;
          node_count--;
          return true;
        }
        temp = temp->next;
      }
      return false;
    }

    int removeNodesWithValue(const T& value) {
      int count = 0;
      Node* current = head;
      while (current) {
        Node* nextNode = current->next;
        if (current->value == value) {
          if (current == head) {
            removeHead();
          } else if (current == tail) {
            removeTail();
          } else {
            current->prev->next = current->next;
            current->next->prev = current->prev;
            delete current;
            node_count--;
          }
          count++;
        }
        current = nextNode;
      }
      return count;
    }

  iterator erase(iterator pos) {
      Node* curr = pos.current;
      if (!curr) return end();

      Node* nextNode = curr->next;
      Node* prevNode = curr->prev;

      if (curr == head) {
        head = nextNode;
      }
      else {
        prevNode->next = nextNode;
      }

      if (curr == tail) {
        tail = prevNode;
      }
      else {
        nextNode->prev = prevNode;
      }

      delete curr;
      node_count--;

      return iterator(nextNode);
    }

    void clear() {
      Node* current = head;
      while (current) {
        Node* next = current->next;
        delete current;
        current = next;
      }
      head = nullptr;
      tail = nullptr;
      node_count = 0;
    }

    T& operator[](size_t index) {
      if (index >= node_count) {
        throw std::out_of_range("Index out of range!");
      }
      Node* current = head;
      for (size_t i = 0; i < index; ++i) {
        current = current->next;
      }
      return current->value;
    }

    const T& operator[](size_t index) const {
      if (index >= node_count) {
        throw std::out_of_range("Index out of range!");
      }
      Node* current = head;
      for (size_t i = 0; i < index; ++i) {
        current = current->next;
      }
      return current->value;
    }

    bool operator==(const Doubly_Linked_List& other) const {
      if (node_count != other.node_count) return false;
      Node* temp1 = head;
      Node* temp2 = other.head;
      while (temp1) {
        if (temp1->value != temp2->value) return false;
        temp1 = temp1->next;
        temp2 = temp2->next;
      }
      return true;
    }

    bool operator!=(const Doubly_Linked_List& other) const {
      return !(*this == other);
    }
};

#endif //DOUBLY_LINKED_LIST_H
