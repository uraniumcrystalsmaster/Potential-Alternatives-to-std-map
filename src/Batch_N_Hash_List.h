#ifndef UMAP_WITH_SORTER_H
#define UMAP_WITH_SORTER_H
#include <vector>
#include "Doubly_Linked_Hash_Map.h"
#include "Radix_Sort.h"
template<typename Key, typename Value>
class Batch_N_Hash_List : public Doubly_Linked_Hash_Map<Key, Value>{
    public:
        static constexpr Key NULL_KEY = std::numeric_limits<Key>::max();
        using Doubly_Linked_Hash_Map<Key, Value>::Doubly_Linked_Hash_Map;
        using omap_iter = typename Doubly_Linked_Hash_Map<Key, Value>::iterator;
        Batch_N_Hash_List() : Doubly_Linked_Hash_Map<Key, Value>() {}

        void sort_keys(){
            // Convert umap to vector
            std::vector<std::pair<const Key, Value>> sorted_pairs;
            auto current_key = this->getHead();
            while (current_key != NULL_KEY && sorted_pairs.size() < this->nodeCount()) {
                auto it = this->find(current_key);
                sorted_pairs.push_back({it->first, it->second.value});
                current_key = it->second.next;
            }
            // Sort
            radix_sort(sorted_pairs.begin(),sorted_pairs.end());

            // Convert vector to umap
            rebuild_sorted_links(sorted_pairs);
        }

    /*
        void sort_values(){
            // Convert umap to vector
            std::vector<std::pair<const Key, Value>> sorted_pairs;
            auto current_key = this->getHead();
            while (current_key != NULL_KEY && sorted_pairs.size() < this->nodeCount()) {
                auto it = this->find(current_key);
                sorted_pairs.push_back({it->first, it->second.value});
                current_key = it->second.next;
            }
            // Sort
            radix_sort(sorted_pairs.begin(),sorted_pairs.end());

            // Convert vector to umap
            rebuild_sorted_links(sorted_pairs);
        }
        */

    void rebuild_sorted_links(const std::vector<std::pair<const Key, Value>>& sorted_pairs) {
            if (sorted_pairs.empty()) {
                this->head = NULL_KEY;
                this->tail = NULL_KEY;
                this->node_count = 0;
                return;
            }

            size_t N = sorted_pairs.size();
            this->head = sorted_pairs.front().first;
            this->tail = sorted_pairs.back().first;

            for (size_t i = 0; i < N; ++i) {
                Key current_key = sorted_pairs[i].first;

                auto it = this->find(current_key);

                if (i < N - 1) {
                    Key next_key = sorted_pairs[i + 1].first;
                    it->second.next = next_key;
                } else {
                    it->second.next = NULL_KEY;
                }

                if (i > 0) {
                    Key prev_key = sorted_pairs[i - 1].first;
                    it->second.prev = prev_key;
                } else {
                    it->second.prev = NULL_KEY;
                }
            }
        }

        omap_iter predecessor(const Key& key) {
            this->sort_keys();
            omap_iter node = this->find(key);
            if(node == this->end()) {
				omap_iter pred = this->end();
                for(omap_iter iter = this->begin(); iter != this->end(); ++iter) {
                    if(key < iter->first) {
                        return pred;
                    }
					pred = iter;
                }
				return pred;
            }
            return node;
        }

        omap_iter successor(const Key& key) {
            this->sort_keys();
            omap_iter node = this->find(key);
            if(node == this->end()) {
                for(omap_iter iter = this->begin(); iter != this->end(); ++iter) {
                    if(key < iter->first) {
                        return iter;
                    }
                }
            }
            return node;
        }


        std::vector<omap_iter> batch_predecessors(std::vector<Key>& keys) {
            this->sort_keys();
            radix_sort(keys.begin(),keys.end());

            std::vector<omap_iter> results;
            results.reserve(keys.size());

            omap_iter map_iter = this->begin();
            omap_iter curr_pred = this->end();

            for(size_t i = 0; i < keys.size(); i++){
                while (map_iter != this->end() and map_iter->first <= keys[i]) {
                    curr_pred = map_iter;
                    ++map_iter;
                }
                results.push_back(curr_pred);
            }
            return results;
        }

        std::vector<omap_iter> batch_successors(std::vector<Key>& keys) {
            this->sort_keys();
            radix_sort(keys.begin(),keys.end());

            std::vector<omap_iter> results;
            results.reserve(keys.size());

            omap_iter map_iter = this->begin();

            for(size_t i = 0; i < keys.size(); i++){
                while (map_iter != this->end() and map_iter->first < keys[i]) {
                    ++map_iter;
                }
                results.push_back(map_iter);
            }
            return results;
        }
};
#endif //UMAP_WITH_SORTER_H
