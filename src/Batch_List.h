//
// Created by urani on 11/21/2025.
//

#ifndef BATCH_LIST_H
#define BATCH_LIST_H
#include <algorithm>
//#include <limits>
#include <utility>
#include <vector>
#include "Doubly_Linked_List.h"
#include "Radix_Sort.h"
template<typename Key, typename Value>
class Batch_List : public Doubly_Linked_List<std::pair<Key, Value>>{
    public:
        using Pair = std::pair<Key, Value>;
        using DLL = Doubly_Linked_List<Pair>;
        using list_iter = typename DLL::iterator;
    private:
        void sort_keys(){
            radix_sort(*this, [](const Pair& p) { return p.first; });
        }
    public:
        Batch_List() : DLL() {}

        void insert(const std::pair<Key, Value>& map_pair) {
            this->addHead(map_pair);
            this->sort_keys();
        }

        void insert(const Key& key, const Value& value) {
            this->addHead({key, value});
            this->sort_keys();
        }

        list_iter find(const Key& key){
            for(auto iter = this->begin(); iter != this->end(); ++iter){
                if(key == iter->first){
                    return iter;
                }
            }
            return this->end();
        }

        list_iter predecessor(const Key& key) {
            this->sort_keys();
            list_iter curr_node = this->begin();
            list_iter prev_node = this->end();
            while (curr_node != this->end()) {
                if (curr_node->first >= key) {
                    return prev_node;
                }
                prev_node = curr_node;
                ++curr_node;
            }
            return prev_node;
        }

        list_iter successor(const Key& key) {
            this->sort_keys();
            list_iter curr_node = this->begin();
            while (curr_node != this->end()) {
                if (curr_node->first > key) {
                    return curr_node;
                }
                ++curr_node;
            }
            return this->end();
        }

        void erase_key(const Key& key){
            for(auto iter = this->begin(); iter != this->end(); ++iter){
                if(iter->first == key){
                    this->erase(iter);
                }
            }
        }

        template<typename InputIter>
        void batch_insert(InputIter begin, InputIter end) {
            while(begin != end){
                this->addHead(*begin);
                ++begin;
            }
            this->sort_keys();
        }

        void batch_insert(const std::vector<Key>& keys) {
            batch_insert(keys.begin(), keys.end());
        }

        std::vector<list_iter> batch_find(std::vector<Key> keys){
            this->sort_keys();
            radix_sort(keys.begin(),keys.end(), [](const Key& key) { return key; });

            std::vector<list_iter> results;
            results.reserve(keys.size());

            list_iter list_iter = this->begin();

            for(size_t i = 0; i < keys.size(); i++){
                while (list_iter != this->end() and list_iter->first < keys[i]) {
                    ++list_iter;
                }
                if (list_iter != this->end() and list_iter->first == keys[i]) {
                    results.push_back(list_iter); // Found key
                }
                else {
                    results.push_back(this->end()); // Didn't find key
                }
            }
            return results;
        }

        std::vector<list_iter> batch_predecessors(std::vector<Key> keys) {
            this->sort_keys();
            radix_sort(keys.begin(),keys.end(), [](const Key& key) { return key; });

            std::vector<list_iter> results;
            results.reserve(keys.size());

            list_iter curr_node = this->begin();
            list_iter potential_pred = this->end();

            for(size_t i = 0; i < keys.size(); i++){
                while (curr_node != this->end() and curr_node->first < keys[i]) {
                    potential_pred = curr_node;
                    ++curr_node;
                }
                results.push_back(potential_pred);
            }
            return results;
        }

        std::vector<list_iter> batch_successors(std::vector<Key> keys) {
            this->sort_keys();
            radix_sort(keys.begin(),keys.end(), [](const Key& key) { return key; });

            std::vector<list_iter> results;
            results.reserve(keys.size());

            list_iter curr_node = this->begin();

            for(size_t i = 0; i < keys.size(); i++){
                while (curr_node != this->end() and curr_node->first <= keys[i]) {
                    ++curr_node;
                }
                results.push_back(curr_node);
            }
            return results;
        }

        template<typename InputIter>
        void batch_erase(InputIter begin, InputIter end) {
            std::vector<Key> keys2erase(begin, end);
            if (keys2erase.empty()) return;

            this->sort_keys();
            radix_sort(keys2erase.begin(), keys2erase.end(), [](const Key& key) { return key; });

            // Remove duplicates
            auto last = std::unique(keys2erase.begin(), keys2erase.end());
            keys2erase.erase(last, keys2erase.end());

            // Start in-place removal
            auto list_it = this->begin();
            auto key_it = keys2erase.begin();

            while (list_it != this->end() and key_it != keys2erase.end()) {
                if (list_it->first < *key_it) {
                    ++list_it;
                }
                else if (list_it->first == *key_it) {
                    list_it = this->erase(list_it);
                    ++key_it;
                }
                else {
                    ++key_it;
                }
            }
        }

        void batch_erase(const std::vector<Key>& keys) {
            batch_erase(keys.begin(), keys.end());
        }
};
#endif //BATCH_LIST_H
