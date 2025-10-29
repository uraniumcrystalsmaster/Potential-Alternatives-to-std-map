#ifndef X_FAST_TRIE_H
#define X_FAST_TRIE_H
#include <cmath>
#include <limits>
#include <vector>
#include "submodules/FunnelHashMap/src/Funnel_Hash_Map.h"
#include "Ordered_Hash_Map.h"

template<typename Value>
class XFastTrie{
	constexpr static size_t NULL_KEY = std::numeric_limits<size_t>::max();
	using iter_upper_levels = Funnel_Hash_Map<size_t, size_t>::iterator;
	using const_iter_upper_levels = Funnel_Hash_Map<size_t, size_t>::const_iterator;
	using iter_lowest_level = typename Ordered_Hash_Map<size_t, Value>::iterator;
	using const_iter_lowest_level = typename Ordered_Hash_Map<size_t, Value>::const_iterator;
    Ordered_Hash_Map<size_t,Value> lowest_level;
    std::vector<Funnel_Hash_Map<size_t,size_t>> upper_levels;
    size_t bit_count;

    public:
		// Patched by Kwan
		// explicit XFastTrie(size_t N) : lowest_level(N), bit_count(std::numeric_limits<size_t>::digits) {
		// 	upper_levels.clear();
		// 	upper_levels.reserve(bit_count - 1);
		// 	for(size_t i = 0; i < bit_count - 1; i++){
		// 		size_t shift = std::min<size_t>(i + 1, std::numeric_limits<size_t>::digits - 1);
		// 		size_t expect = std::min(N, static_cast<size_t>(1) << shift);
		// 		size_t alloc = expect < 8 ? 8 : expect;
		// 		upper_levels.emplace_back(alloc, 0.35);
		// 	}
		// }

		// + (std::is_signed<size_t>::value ? 1 : 0); //add one for signed values
		explicit XFastTrie(size_t N) : lowest_level(N), bit_count(std::numeric_limits<size_t>::digits) {
            upper_levels.reserve(bit_count-1);
            for (size_t i = 0; i < bit_count - 1; i++) {
                constexpr size_t one = 1;
                upper_levels.emplace_back(std::min(N, (one << (i+one))));
            }
        }

		// Patched by Kwan (faster than the old insertion)
		bool insert(const size_t& key, const Value& value){
			if (lowest_level.contains(key)) return false;

			if (lowest_level.empty()) {
				for (size_t i = 0; i < bit_count - 1; i++) {
					size_t parent_key = key >> (bit_count - 1 - i);
					upper_levels[i].emplace(parent_key, key);
				}

				lowest_level.addHead(key, value);
				return true;
			}

			const size_t head_key = lowest_level.getHead(), tail_key = lowest_level.getTail();
			if (key < head_key) lowest_level.addHead(key, value);
			else if (key > tail_key) lowest_level.addTail(key, value);
			else {
				auto pred_it = predecessor(key);
				if (pred_it == lowest_level.end()) {
					auto succ_it = successor(key);
					lowest_level.insertBefore(key, value, succ_it.key());
				} else lowest_level.insertAfter(key, value, pred_it.key());
			}

			const size_t lcpli = findLongestCommonPrefixLevelIndex(key);

			for (size_t i = lcpli + 1; i < bit_count - 1; ++i) {
				size_t parent_key = key >> (bit_count - 1 - i);
				upper_levels[i].emplace(parent_key, key);
			}

			for (size_t i = 0; i <= lcpli; i++) {
				const size_t ancestor_level = (lcpli - i);
				size_t ancestor_key = key >> (bit_count - 1 - ancestor_level);

				const iter_upper_levels ancestor_node = upper_levels[ancestor_level].find(ancestor_key);
				size_t left_child_key = ancestor_key << 1, right_child_key = (ancestor_key << 1) + 1;

				bool has_left_child, has_right_child;
				if (ancestor_level == bit_count - 2) {
					has_left_child = lowest_level.contains(left_child_key);
					has_right_child = lowest_level.contains(right_child_key);
				} else {
					has_left_child = upper_levels[ancestor_level + 1].contains(left_child_key);
					has_right_child = upper_levels[ancestor_level + 1].contains(right_child_key);
				}

				if (has_left_child && has_right_child) ancestor_node->second = NULL_KEY;
				else if (!has_left_child) {
					const size_t old_smallest = ancestor_node->second;
					if (old_smallest == NULL_KEY || key < old_smallest) ancestor_node->second = key;
				} else {
					const size_t old_largest = ancestor_node->second;
					if (old_largest == NULL_KEY || key > old_largest) ancestor_node->second = key;
				}
			}

			return true;
		}

   //      bool insert(const size_t& key, Value& value){
			// // Don't enter duplicates into the trie
			// if(lowest_level.contains(key)){
			// 	return false;
			// }
   //
   //          if(lowest_level.empty()){
   //              for(size_t i = 0; i < bit_count-1; i++){
			// 		size_t parent_key = key >> (bit_count - 1 - i);
   //                  upper_levels[i].emplace(parent_key, key);
   //              }
			// 	lowest_level.addHead(key,value);
   //              return true;
   //          }
			// // Add nodes to trie
			// const size_t lcpli = findLongestCommonPrefixLevelIndex(key);
			// size_t descendant_key = upper_levels[lcpli].find(key >> ((bit_count-1) - lcpli))->second;
			// if(key < descendant_key){
			// 	lowest_level.insertBefore(key,value,descendant_key);
			// }
			// else{
			// 	lowest_level.insertAfter(key,value,descendant_key);
			// }
			// for(size_t i = lcpli + 1; i < bit_count-1; ++i){
			// 		size_t parent_key = key >> (bit_count - 1 - i);
   //                  upper_levels[i].emplace(parent_key, key);
   //          }
   //
			// // Update descendants
			// for(size_t i = 0; i <= lcpli; i++){
			// 	const size_t ancestor_level = (lcpli - i);
			// 	size_t ancestor_key = key >> (bit_count - 1 - ancestor_level);
			// 	iter_upper_levels ancestor_node = upper_levels[ancestor_level].find(ancestor_key);
			// 	size_t left_child_key = ancestor_key << 1;
			// 	size_t right_child_key = (ancestor_key << 1)+1;
			// 	bool has_left_child;
			// 	bool has_right_child;
			// 	if(ancestor_level == bit_count-2){
			// 		has_left_child = lowest_level.contains(left_child_key);
   //     				has_right_child = lowest_level.contains(right_child_key);
			// 	}
			// 	else{
			// 		has_left_child = upper_levels[ancestor_level + 1].contains(left_child_key);
			// 		has_right_child = upper_levels[ancestor_level + 1].contains(right_child_key);
			// 	}
   //
			// 	if(has_left_child and has_right_child){
			// 		ancestor_node->second = NULL_KEY;
			// 	}
			// 	else if(!has_left_child){
			// 		const size_t old_smallest = ancestor_node->second;
			// 		if(old_smallest == NULL_KEY or key < old_smallest){
			// 			ancestor_node->second = key;
			// 		}
			// 	}
			// 	else if(!has_right_child){
			// 		const size_t old_largest = ancestor_node->second;
			// 		if(old_largest == NULL_KEY or key > old_largest){
			// 			ancestor_node->second = key;
			// 		}
			// 	}
			// 	else{
			// 		throw std::runtime_error("Error: Need to fix trie's code. This shouldn't happen.");
			// 	}
   //          }
			// return true;
   //      }

		// Patched by Kwan
		bool remove(const size_t& key) {
			iter_lowest_level node_to_remove = lowest_level.find(key);
			if (node_to_remove == lowest_level.end()) return false;

			size_t succ = NULL_KEY, pred = NULL_KEY;
			if (node_to_remove != lowest_level.begin()) pred = std::prev(node_to_remove).key();

			auto it_next = std::next(node_to_remove);
			if (it_next != lowest_level.end()) succ = it_next.key();

			lowest_level.remove(key);
			if (lowest_level.empty()) {
				for (auto & upper_level : upper_levels) upper_level.clear();
				return true;
			}

			for (size_t i = 0; i < bit_count - 1; i++) {
				const size_t ancestor_level = (bit_count - 2 - i);
				size_t ancestor_key = key >> (bit_count - 1 - ancestor_level);

				const iter_upper_levels ancestor_node = upper_levels[ancestor_level].find(ancestor_key);

				size_t left_child_key = ancestor_key << 1;
				size_t right_child_key = (ancestor_key << 1) + 1;

				bool has_left_child, has_right_child;
				if (ancestor_level == bit_count - 2) {
					if (key == left_child_key) {
						has_left_child = false;
						has_right_child = lowest_level.contains(right_child_key);
					} else {
						has_left_child = lowest_level.contains(left_child_key);
						has_right_child = false;
					}
				} else {
					has_left_child = upper_levels[ancestor_level + 1].contains(left_child_key);
					has_right_child = upper_levels[ancestor_level + 1].contains(right_child_key);
				}

				if (has_left_child and has_right_child) {
					ancestor_node->second = NULL_KEY;
					return true;
				}

				if (!has_left_child and !has_right_child) upper_levels[ancestor_level].erase(ancestor_key);
				else if(!has_left_child) ancestor_node->second = succ;
				else ancestor_node->second = pred;
			}

			return true;
		}

		// bool remove(const size_t& key){
		// 	iter_lowest_level node2remove = lowest_level.find(key);
		// 	if(node2remove == lowest_level.end()){
		// 		return false;
		// 	}
		// 	const size_t succ = node2remove->second.next;
		// 	const size_t pred = node2remove->second.prev;
		// 	lowest_level.remove(key);
		// 	if(lowest_level.empty()){
		// 		for(size_t i = 0; i < upper_levels.size(); i++){
		// 			upper_levels[i].clear();
		// 		}
		// 		return true;
		// 	}
		// 	for(size_t i = 0; i < bit_count-1; i++){
		// 		const size_t ancestor_level = (bit_count - 2 - i);
		// 		size_t ancestor_key = key >> (bit_count - 1 - ancestor_level);
		// 		iter_upper_levels ancestor_node = upper_levels[ancestor_level].find(ancestor_key);
		// 		size_t left_child_key = ancestor_key << 1;
		// 		size_t right_child_key = (ancestor_key << 1)+1;
		// 		bool has_left_child;
		// 		bool has_right_child;
		// 		if(ancestor_level == bit_count-2){
		// 			if (key == left_child_key) {
  //               		has_left_child = false;
  //               		has_right_child = lowest_level.contains(right_child_key);
  //          	 		}
		// 			else {
  //               		has_left_child = lowest_level.contains(left_child_key);
  //               		has_right_child = false;
  //           		}
		// 		}
		// 		else{
		// 			has_left_child = upper_levels[ancestor_level + 1].contains(left_child_key);
		// 			has_right_child = upper_levels[ancestor_level + 1].contains(right_child_key);
		// 		}
		// 		if(has_left_child and has_right_child){
		// 			ancestor_node->second = NULL_KEY;
		// 			return true;
		// 		}
		// 		else if(!has_left_child and !has_right_child){
		// 			upper_levels[ancestor_level].erase(ancestor_key);
		// 		}
		// 		else if(!has_left_child){
		// 			ancestor_node->second = succ;
		// 		}
		// 		else{ // Does not have right child
		// 			ancestor_node->second = pred;
		// 		}
  //           }
		// 	return true;
		// }

		iter_lowest_level find(const size_t& key){
			return lowest_level.find(key);
		}

		const_iter_lowest_level find(const size_t& key) const{
			return lowest_level.find(key);
		}

		bool contains(const size_t& key){
        	return lowest_level.contains(key);
        }

		bool contains(const size_t& key) const{
        	return lowest_level.contains(key);
        }

		// Patched by Kwan
		iter_lowest_level predecessor(const size_t& key) {
			auto node = lowest_level.find(key);
			if(node == lowest_level.end()){
				const size_t prefix_index = findLongestCommonPrefixLevelIndex(key);
				const size_t prefix_key = key >> (bit_count - 1 - prefix_index);
				size_t desc_key = upper_levels[prefix_index].find(prefix_key)->second;
				auto desc_node = lowest_level.find(desc_key);

				if (key < desc_key) {
					if(desc_node == lowest_level.begin()) return lowest_level.end();
					return std::prev(desc_node);
				}

				return desc_node;
			}

			if (node == lowest_level.begin()) return lowest_level.end();

			return std::prev(node);
		}

		const_iter_lowest_level predecessor(const size_t& key) const {
			auto node = lowest_level.find(key);
			if(node == lowest_level.cend()){
				const size_t prefix_index = findLongestCommonPrefixLevelIndex(key);
				const size_t prefix_key = key >> (bit_count - 1 - prefix_index);
				size_t desc_key = upper_levels[prefix_index].find(prefix_key)->second;
				auto desc_node = lowest_level.find(desc_key);

				if (key < desc_key) {
					if(desc_node == lowest_level.cbegin()) return lowest_level.cend();
					return std::prev(desc_node);
				}

				return desc_node;
			}

			if(node == lowest_level.cbegin()) return lowest_level.cend();

			return std::prev(node);
		}

		iter_lowest_level successor(const size_t& key) {
			auto node = lowest_level.find(key);
			if (node == lowest_level.end()) {
				const size_t prefix_index = findLongestCommonPrefixLevelIndex(key);
				const size_t prefix_key = key >> (bit_count - 1 - prefix_index);
				size_t desc_key = upper_levels[prefix_index].find(prefix_key)->second;
				auto desc_node = lowest_level.find(desc_key);

				if(key > desc_key) return std::next(desc_node);
				return desc_node;
			}

			return std::next(node);
		}

		const_iter_lowest_level successor(const size_t& key) const {
			auto node = lowest_level.find(key);
			if (node == lowest_level.cend()) {
				const size_t prefix_index = findLongestCommonPrefixLevelIndex(key);
				const size_t prefix_key = key >> (bit_count - 1 - prefix_index);
				size_t desc_key = upper_levels[prefix_index].find(prefix_key)->second;
				auto desc_node = lowest_level.find(desc_key);

				if (key > desc_key) return std::next(desc_node);
				return desc_node;
			}

			return std::next(node);
		}

		// iter_lowest_level predecessor(const size_t& key) {
		// 	iter_lowest_level possible_node = lowest_level.find(key);
		// 	if(possible_node == lowest_level.end()){
  //          		const size_t lcpli = findLongestCommonPrefixLevelIndex(key);
		// 		const size_t prefix_key_at_lcpli = key >> ((bit_count-1) - lcpli);
  //       		size_t descendant_key = upper_levels[lcpli].find(prefix_key_at_lcpli)->second;
  //       		iter_lowest_level descendant_node = lowest_level.find(descendant_key);
		// 		if(key < descendant_key){
		// 			if(descendant_node->second.prev == NULL_KEY){
		// 				return lowest_level.end();
		// 			}
		// 			return lowest_level.find(descendant_node->second.prev);
		// 		}
		// 		return descendant_node;
		// 	}
		// 	if(possible_node->second.prev == NULL_KEY){
		// 		return lowest_level.end();
		// 	}
		// 	return lowest_level.find(possible_node->second.prev);
		// }
  //
		// const_iter_lowest_level predecessor(const size_t& key) const{
		// 	const_iter_lowest_level possible_node = lowest_level.find(key);
		// 	if(possible_node == lowest_level.end()){
  //          		size_t lcpli = findLongestCommonPrefixLevelIndex(key);
		// 		size_t prefix_key_at_lcpli = key >> ((bit_count-1) - lcpli);
  //       		size_t descendant_key = upper_levels[lcpli].find(prefix_key_at_lcpli)->second;
  //       		const_iter_lowest_level descendant_node = lowest_level.find(descendant_key);
		// 		if(key < descendant_key){
		// 			if(descendant_node->second.prev == NULL_KEY){
		// 				return lowest_level.end();
		// 			}
		// 			return lowest_level.find(descendant_node->second.prev);
		// 		}
		// 		return descendant_node;
		// 	}
		// 	if(possible_node->second.prev == NULL_KEY){
		// 		return lowest_level.end();
		// 	}
		// 	return lowest_level.find(possible_node->second.prev);
		// }
  //
		// iter_lowest_level successor(const size_t& key){
		// 	iter_lowest_level possible_node = lowest_level.find(key);
		// 	if(possible_node == lowest_level.end()){
  //          		size_t lcpli = findLongestCommonPrefixLevelIndex(key);
		// 		size_t prefix_key_at_lcpli = key >> ((bit_count-1) - lcpli);
  //       		size_t descendant_key = upper_levels[lcpli].find(prefix_key_at_lcpli)->second;
  //       		iter_lowest_level descendant_node = lowest_level.find(descendant_key);
		// 		if(key > descendant_key){
		// 			if(descendant_node->second.next == NULL_KEY){
		// 				return lowest_level.end();
		// 			}
		// 			return lowest_level.find(descendant_node->second.next);
		// 		}
		// 		return descendant_node;
		// 	}
		// 	if(possible_node->second.next == NULL_KEY){
		// 		return lowest_level.end();
		// 	}
		// 	return lowest_level.find(possible_node->second.next);
  //       }
  //
		// const_iter_lowest_level successor(const size_t& key) const{
		// 	const_iter_lowest_level possible_node = lowest_level.find(key);
		// 	if(possible_node == lowest_level.end()){
  //          		const size_t lcpli = findLongestCommonPrefixLevelIndex(key);
		// 		const size_t prefix_key_at_lcpli = key >> ((bit_count-1) - lcpli);
  //       		size_t descendant_key = upper_levels[lcpli].find(prefix_key_at_lcpli)->second;
  //       		const_iter_lowest_level descendant_node = lowest_level.find(descendant_key);
		// 		if(key > descendant_key){
		// 			if(descendant_node->second.next == NULL_KEY){
		// 				return lowest_level.end();
		// 			}
		// 			return lowest_level.find(descendant_node->second.next);
		// 		}
		// 		return descendant_node;
		// 	}
		// 	if(possible_node->second.next == NULL_KEY){
		// 		return lowest_level.end();
		// 	}
		// 	return lowest_level.find(possible_node->second.next);
		// }

        size_t findLongestCommonPrefixLevelIndex(const size_t& key){
            size_t target_prefix_level_i = 0;
            size_t low = 0;
            size_t high = bit_count-2;
            while(low <= high){
                size_t midpoint = low + (high - low) / 2;
                size_t curr_prefix = key >> (bit_count - (midpoint + 1));
                if(upper_levels[midpoint].contains(curr_prefix)){
                    low = midpoint + 1;
                    target_prefix_level_i = midpoint;
                }
                else {
                    if(high == 0){ break; } // Prevent underflow
                    high = midpoint - 1;
                }
            }
            return target_prefix_level_i;
        }

		size_t findLongestCommonPrefixLevelIndex(const size_t& key) const{
            size_t target_prefix_level_i = 0;
            size_t low = 0;
            size_t high = bit_count-2;
            while(low <= high){
                const size_t midpoint = low + (high - low) / 2;
                size_t curr_prefix = key >> (bit_count - (midpoint + 1));
                if(upper_levels[midpoint].contains(curr_prefix)){
                    low = midpoint + 1;
                    target_prefix_level_i = midpoint;
                }
                else {
                    if(high == 0){ break; } //prevent underflow
                    high = midpoint - 1;
                }
            }
            return target_prefix_level_i;
        }

		iter_lowest_level begin() {
			return lowest_level.begin();
		}

		const_iter_lowest_level cbegin() const {
			return lowest_level.cbegin();
		}

		iter_lowest_level end() {
			return lowest_level.end();
		}

		const_iter_lowest_level cend() const {
			return lowest_level.cend();
		}
};
#endif //X_FAST_TRIE_H
