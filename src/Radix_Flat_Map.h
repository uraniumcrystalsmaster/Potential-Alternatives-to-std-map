//
// Created by urani on 10/30/2025.
//
#ifndef RADIX_FLAT_MAP_H
#define RADIX_FLAT_MAP_H
#include "Radix_Sort.h"

template<typename Key, typename Value>
class Radix_Flat_Map{
    std::vector<std::pair<Key, Value>> radix_flat_map;
    public:
    using iterator = typename std::vector<std::pair<Key, Value>>::iterator;
    using const_iterator = typename std::vector<std::pair<Key, Value>>::const_iterator;

    Radix_Flat_Map() = default;

    template<typename input_iterator>
    explicit Radix_Flat_Map(input_iterator begin, input_iterator end)
    : radix_flat_map(begin, end){
        radix_sort(radix_flat_map.begin(),radix_flat_map.end(), [](const auto& p) { return p.first; });
        remove_duplicates(radix_flat_map, [](const auto& p) { return p.first; });
    }

    template<typename AnyVector, typename Getter>
    void remove_duplicates(AnyVector& vec2remdups, Getter get_key) {
        if (vec2remdups.empty()) return;

        auto write_pos = vec2remdups.begin();
        auto read_pos = vec2remdups.begin() + 1;

        while (read_pos != vec2remdups.end()) {
            if (get_key(*write_pos) != get_key(*read_pos)) {
                ++write_pos;
                if (write_pos != read_pos) {
                    *write_pos = std::move(*read_pos);
                }
            }
            ++read_pos;
        }

        vec2remdups.erase(write_pos + 1, vec2remdups.end());
    }

    size_t lower_bound_binary_search(const Key& key2find) {
        /* same as upper bound binary search except at if(key2find > radix_flat_map[midpoint].first){ */
        size_t low = 0;
        size_t high = radix_flat_map.size();
        while(low < high){
            size_t midpoint = low + (high - low) / 2;
            if(key2find > radix_flat_map[midpoint].first){
                low = midpoint + 1;
            }
            else{
                high = midpoint;
            }
        }
        return low;
    }

    size_t upper_bound_binary_search(const Key& key2find) {
        /* same as lower bound binary search except at if(key2find >= radix_flat_map[midpoint].first){ */
        size_t low = 0;
        size_t high = radix_flat_map.size();
        while(low < high){
            size_t midpoint = low + (high - low) / 2;
            if(key2find >= radix_flat_map[midpoint].first){
                low = midpoint + 1;
            }
            else{
                high = midpoint;
            }
        }
        return low;
    }

    Value& operator[](const Key& key) {
        size_t pos = lower_bound_binary_search(key);

        if (pos < radix_flat_map.size() and radix_flat_map[pos].first == key) {
            return radix_flat_map[pos].second;
        }

        auto it = radix_flat_map.emplace(radix_flat_map.begin() + pos, key, Value{});
        return it->second;
    }

    bool insert(const std::pair<Key, Value>& map_pair) {
        return insert(map_pair.first, map_pair.second);
    }

    bool insert(const Key& key, const Value& value){
        size_t pos = lower_bound_binary_search(key);

        //check if duplicate key
        if(pos < radix_flat_map.size() and key == radix_flat_map[pos].first) {
            return false;
        }

        //emplace (insert but more efficient) at position
        radix_flat_map.emplace(radix_flat_map.begin() + pos, key, value);
        return true;
    }

    bool erase(const Key& key){
        size_t pos = lower_bound_binary_search(key);

        //check if key doesn't exist
        if(pos >= radix_flat_map.size() or key != radix_flat_map[pos].first) {
            return false;
        }

        radix_flat_map.erase(radix_flat_map.begin() + pos);
        return true;
    }

    iterator find(const Key& key) {
        size_t pos = lower_bound_binary_search(key);
        if (pos < radix_flat_map.size() and radix_flat_map[pos].first == key) {
            return radix_flat_map.begin() + pos;
        }
        return this->end();
    }

    iterator predecessor(const Key& key){
        size_t pos = lower_bound_binary_search(key);

        //no predecessor
        if(pos == 0) {
            return this->end();
        }

        return radix_flat_map.begin() + (pos - 1);
    }

    iterator successor(const Key& key){
         size_t pos = upper_bound_binary_search(key);

        //no successor
        if(pos >= radix_flat_map.size()) {
            return this->end();
        }

        return radix_flat_map.begin() + pos;
    }

    void reserve(const size_t N) {
        radix_flat_map.reserve(N);
    }

    template<typename InputIter>
    void insert_batch(InputIter begin, InputIter end) {
        radix_flat_map.insert(radix_flat_map.end(), begin, end);
        radix_sort(radix_flat_map.begin(),radix_flat_map.end(), [](const auto& p) { return p.first; });
        remove_duplicates(radix_flat_map, [](const auto& p) { return p.first; });
    }

    template<typename InputIter>
    void erase_batch(InputIter begin, InputIter end) {
        std::vector<Key> keys2erase(begin, end);
        radix_sort(keys2erase.begin(),keys2erase.end(), [](const Key& key) { return key; });
        remove_duplicates(keys2erase, [](const Key& key) { return key; });

        //Perform the 2-pointer filter algorithm
        auto read_iter = radix_flat_map.begin();
        auto write_iter = radix_flat_map.begin();
        auto key2erase_iter = keys2erase.begin();

        while (read_iter != radix_flat_map.end()) {
            if (key2erase_iter == keys2erase.end()) {
                if (write_iter != read_iter) {
                    *write_iter = std::move(*read_iter);
                }
                ++write_iter;
                ++read_iter;
            }
            else if (read_iter->first < *key2erase_iter) {
                if (write_iter != read_iter) {
                    *write_iter = std::move(*read_iter);
                }
                ++write_iter;
                ++read_iter;
            }
            else if (read_iter->first == *key2erase_iter) {
                ++read_iter;
                ++key2erase_iter;
            }
            else {
                ++key2erase_iter;
            }
        }

        radix_flat_map.erase(write_iter, radix_flat_map.end());
    }

    // Iterator methods
    iterator begin() {
        return radix_flat_map.begin();
    }

    iterator end() {
        return radix_flat_map.end();
    }

    const_iterator begin() const {
        return radix_flat_map.begin();
    }

    const_iterator end() const {
        return radix_flat_map.end();
    }

    const_iterator cbegin() const {
        return radix_flat_map.cbegin();
    }

    const_iterator cend() const {
        return radix_flat_map.cend();
    }
};
#endif //RADIX_FLAT_MAP_H
