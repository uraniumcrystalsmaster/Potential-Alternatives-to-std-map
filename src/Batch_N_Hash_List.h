#ifndef UMAP_WITH_SORTER_H
#define UMAP_WITH_SORTER_H
#include <vector>
#include <limits>
#include "Doubly_Linked_Hash_Map.h"
#include "Radix_Sort.h"
template<typename Key, typename Value>
class Batch_N_Hash_List : public Doubly_Linked_Hash_Map<Key, Value>{
    public:
        static constexpr Key NULL_KEY = std::numeric_limits<Key>::max();
        using Doubly_Linked_Hash_Map<Key, Value>::Doubly_Linked_Hash_Map;
        using omap_iter = typename Doubly_Linked_Hash_Map<Key, Value>::iterator;
        using NodeProps = typename Doubly_Linked_Hash_Map<Key, Value>::NodeProps;
        Batch_N_Hash_List() : Doubly_Linked_Hash_Map<Key, Value>() {}

        void sort_keys(){
            // Convert umap to vector
            std::vector<std::pair<Key, Value>> sorted_pairs;
            for (auto iter = this->begin(); iter != this->end(); ++iter) {
                sorted_pairs.push_back({iter->first, iter->second});
            }

            // Sort
            radix_sort(sorted_pairs.begin(),sorted_pairs.end(), [](const auto& p) { return p.first; });

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

        void rebuild_sorted_links(const std::vector<std::pair<Key, Value>>& sorted_pairs) {
            if (sorted_pairs.empty()) {
                this->head = NULL_KEY;
                this->tail = NULL_KEY;
                this->node_count = 0;
                return;
            }

            size_t N = sorted_pairs.size();
            this->head = sorted_pairs.front().first;
            this->tail = sorted_pairs.back().first;
            this->node_count = N;

            for (size_t i = 0; i < N; ++i) {
                Key current_key = sorted_pairs[i].first;

                // Get the NodeProps struct directly from umap
                NodeProps& node = this->umap.find(current_key)->second;

                if (i < N - 1) {
                    Key next_key = sorted_pairs[i + 1].first;
                    node.next = next_key;
                } else {
                    node.next = NULL_KEY;
                }

                if (i > 0) {
                    Key prev_key = sorted_pairs[i - 1].first;
                    node.prev = prev_key;
                } else {
                    node.prev = NULL_KEY;
                }
            }
        }

        omap_iter predecessor(const Key& key) {
            this->sort_keys();
            omap_iter node = this->find(key);
            if(node == this->end()) {
                omap_iter successor_node = this->end();
                for(omap_iter iter = this->begin(); iter != this->end(); ++iter) {
                    if(key < iter.key()) {
                        successor_node = iter; // Found the crossover
                        break;                 // <-- Exit the loop
                    }
                }

                if (successor_node == this->end()) {
                    return this->find(this->getTail());
                }

                if (successor_node.key() == this->getHead()) {
                    return this->end();
                }

                Key prev_key = this->umap.find(successor_node.key())->second.prev;
                return this->find(prev_key);
            }

            Key prev_key = this->umap.find(node.key())->second.prev;
            if (prev_key == NULL_KEY) {
                return this->end();
            }
            return this->find(prev_key);
        }

        omap_iter successor(const Key& key) {
            this->sort_keys();
            omap_iter node = this->find(key);
            if(node == this->end()) {
                for(omap_iter iter = this->begin(); iter != this->end(); ++iter) {
                    if(key < iter.key()) {
                        return iter;
                    }
                }
                return this->end();
            }
            Key next_key = this->umap.find(node.key())->second.next;
            if (next_key == NULL_KEY) {
                return this->end();
            }
            return this->find(next_key);
        }

        std::vector<omap_iter> batch_predecessors(std::vector<Key>& keys) {
            this->sort_keys();
            radix_sort(keys.begin(),keys.end(), [](const Key& key) { return key; });

            std::vector<omap_iter> results;
            results.reserve(keys.size());

            omap_iter map_iter = this->begin();
            omap_iter curr_pred = this->end();

            for(size_t i = 0; i < keys.size(); i++){
                while (map_iter != this->end() and map_iter.key() < keys[i]) {
                    curr_pred = map_iter;
                    ++map_iter;
                }
                results.push_back(curr_pred);
            }
            return results;
        }

        std::vector<omap_iter> batch_successors(std::vector<Key>& keys) {
            this->sort_keys();
            radix_sort(keys.begin(),keys.end(), [](const Key& key) { return key; });

            std::vector<omap_iter> results;
            results.reserve(keys.size());

            omap_iter map_iter = this->begin();

            for(size_t i = 0; i < keys.size(); i++){
                while (map_iter != this->end() and map_iter.key() <= keys[i]) {
                    ++map_iter;
                }
                results.push_back(map_iter);
            }
            return results;
        }
};
#endif //UMAP_WITH_SORTER_H
