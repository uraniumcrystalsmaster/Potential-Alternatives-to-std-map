#ifndef X_FAST_TRIE_H
#define X_FAST_TRIE_H
#include <cmath>
#include <limits>
#include <vector>
//#include <unordered_map>
#include "Funnel_Hash_Map.h"
#include "Doubly_Linked_Hash_Map.h"

template<typename Key, typename Value>
class XFastTrie{
	static_assert(std::is_integral<Key>::value, "Key must be an int or uint type");
	constexpr static size_t NULL_KEY = std::numeric_limits<size_t>::max();
	using iter_upper_levels = Funnel_Hash_Map<size_t, size_t>::iterator;
	using const_iter_upper_levels = Funnel_Hash_Map<size_t, size_t>::const_iterator;
	//using iter_upper_levels = std::unordered_map<size_t, size_t>::iterator;
	//using const_iter_upper_levels = std::unordered_map<size_t, size_t>::const_iterator;
	using iter_lowest_level = typename Doubly_Linked_Hash_Map<size_t, Value>::iterator;
	using const_iter_lowest_level = typename Doubly_Linked_Hash_Map<size_t, Value>::const_iterator;
    Doubly_Linked_Hash_Map<size_t,Value> lowest_level;
    std::vector<Funnel_Hash_Map<size_t,size_t>> upper_levels;
	//std::vector<std::unordered_map<size_t,size_t>> upper_levels;
    size_t bit_count;

	// These two functions convert between the input key type and a type that the trie can sort effectively

	inline size_t key2Internal(const Key& key) const {
		if (std::is_signed<Key>::value) {
			// Flip the sign bit
			constexpr size_t sign_bit = size_t(1) << (sizeof(Key) * 8 - 1);
			return static_cast<size_t>(static_cast<typename std::make_unsigned<Key>::type>(key)) ^ sign_bit;
		}
		else {
			return static_cast<size_t>(key);
		}
	}

	inline Key internal2Key(size_t internal) const {
		if (std::is_signed<Key>::value) {
			// Reverse the sign bit flip
			constexpr size_t sign_bit = size_t(1) << (sizeof(Key) * 8 - 1);
			internal ^= sign_bit;
			return static_cast<Key>(internal);
		}
		else {
			return static_cast<Key>(internal);
		}
	}

    public:
		explicit XFastTrie(size_t N) : lowest_level(N), bit_count(sizeof(Key) * 8) {
            upper_levels.reserve(bit_count-1);
            for (size_t i = 0; i < bit_count - 1; i++) {
                constexpr size_t one = 1;
                upper_levels.emplace_back(std::min(N, (one << (i+one))));
            }
        }

		// Patched by Kwan (faster than the old insertion) and patched again by Urani (bug fixes)
		bool insert(const Key& key, const Value& value){
			const size_t internal_key = key2Internal(key);

			if (lowest_level.find(internal_key) != lowest_level.end()) return false;

			if (lowest_level.empty()) {
				for (size_t i = 0; i < bit_count - 1; i++) {
					size_t parent_key = internal_key >> (bit_count - 1 - i);
					upper_levels[i].emplace(parent_key, internal_key);
				}

				lowest_level.addHead(internal_key, value);
				return true;
			}

			const size_t head_key = lowest_level.getHead(), tail_key = lowest_level.getTail();
			if (internal_key < head_key) lowest_level.addHead(internal_key, value);
			else if (internal_key > tail_key) lowest_level.addTail(internal_key, value);
			else {
				auto pred_it = predecessorInternal(internal_key);
				if (pred_it == lowest_level.end()) {
					auto succ_it = successorInternal(internal_key);
					if(succ_it == lowest_level.end()) {
						lowest_level.addTail(internal_key, value);
					}
					else {
						lowest_level.insertBefore(internal_key, value, succ_it.key());
					}
				} else lowest_level.insertAfter(internal_key, value, pred_it.key());
			}

			const size_t lcpli = findLongestCommonPrefixLevelIndex(internal_key);

			for (size_t i = lcpli + 1; i < bit_count - 1; ++i) {
				size_t parent_key = internal_key >> (bit_count - 1 - i);
				upper_levels[i].emplace(parent_key, internal_key);
			}

			if(lcpli == NULL_KEY) { return true; };

			for (size_t i = 0; i <= lcpli; i++) {
				const size_t ancestor_level = (lcpli - i);
				size_t ancestor_key = internal_key >> (bit_count - 1 - ancestor_level);

				const iter_upper_levels ancestor_node = upper_levels[ancestor_level].find(ancestor_key);
				if (ancestor_node == upper_levels[ancestor_level].end()) {
					throw std::runtime_error("Trie corruption: ancestor_node not found.");
				}

				size_t left_child_key = ancestor_key << 1, right_child_key = (ancestor_key << 1) + 1;

				bool has_left_child, has_right_child;
				if (ancestor_level == bit_count - 2) {
					has_left_child = lowest_level.find(left_child_key) != lowest_level.end();
					has_right_child = lowest_level.find(right_child_key) != lowest_level.end();
				}
				else {
					has_left_child = upper_levels[ancestor_level + 1].find(left_child_key) != upper_levels[ancestor_level + 1].end();
					has_right_child = upper_levels[ancestor_level + 1].find(right_child_key) != upper_levels[ancestor_level + 1].end();
				}

				if (has_left_child && has_right_child) {
					ancestor_node->second = NULL_KEY;
				}
				else if (!has_left_child) {
					const size_t old_smallest = ancestor_node->second;
					if (old_smallest == NULL_KEY or internal_key < old_smallest) ancestor_node->second = internal_key;
				}
				else {
					const size_t old_largest = ancestor_node->second;
					if (old_largest == NULL_KEY or internal_key > old_largest) ancestor_node->second = internal_key;
				}
			}

			return true;
		}

		// Patched by Kwan
		bool remove(const Key& key) {
			const size_t internal_key = key2Internal(key);

			iter_lowest_level node_to_remove = lowest_level.find(internal_key);
			if (node_to_remove == lowest_level.end()) return false;

			size_t succ = NULL_KEY, pred = NULL_KEY;
			if (node_to_remove != lowest_level.begin()) pred = std::prev(node_to_remove).key();

			auto it_next = std::next(node_to_remove);
			if (it_next != lowest_level.end()) succ = it_next.key();

			lowest_level.remove(internal_key);
			if (lowest_level.empty()) {
				for (auto & upper_level : upper_levels) upper_level.clear();
				return true;
			}

			for (size_t i = 0; i < bit_count - 1; i++) {
				const size_t ancestor_level = (bit_count - 2 - i);
				size_t ancestor_key = internal_key >> (bit_count - 1 - ancestor_level);

				const iter_upper_levels ancestor_node = upper_levels[ancestor_level].find(ancestor_key);

				size_t left_child_key = ancestor_key << 1;
				size_t right_child_key = (ancestor_key << 1) + 1;

				bool has_left_child, has_right_child;
				if (ancestor_level == bit_count - 2) {
					if (internal_key == left_child_key) {
						has_left_child = false;
						has_right_child = lowest_level.find(right_child_key) != lowest_level.end();
					}
					else {
						has_left_child = lowest_level.find(left_child_key) != lowest_level.end();
						has_right_child = false;
					}
				}
				else {
					has_left_child = upper_levels[ancestor_level + 1].find(left_child_key) != upper_levels[ancestor_level + 1].end();
					has_right_child = upper_levels[ancestor_level + 1].find(right_child_key) != upper_levels[ancestor_level + 1].end();
				}

				if (has_left_child and has_right_child) {
					ancestor_node->second = NULL_KEY;
					return true;
				}

				if (!has_left_child and !has_right_child) {
					upper_levels[ancestor_level].erase(ancestor_key);
				}
				else if(!has_left_child) {
					ancestor_node->second = succ;
				}
				else {
					ancestor_node->second = pred;
				}
			}

			return true;
		}

		iter_lowest_level find(const Key& key){
			return lowest_level.find(key2Internal(key));
		}

		const_iter_lowest_level find(const Key& key) const{
			return lowest_level.find(key2Internal(key));
		}

		bool contains(const Key& key){
        	return lowest_level.contains(key2Internal(key));
        }

		bool contains(const Key& key) const{
        	return lowest_level.contains(key2Internal(key));
        }

	private:
		// Internal predecessor that works with size_t keys
		iter_lowest_level predecessorInternal(const size_t& key) {
          auto node = lowest_level.find(key);
          if(node != lowest_level.end()){
             if (node == lowest_level.begin()) return lowest_level.end();
             return std::prev(node);
          }

          // Key not in list, find its logical position via trie
			size_t level_index = findLongestCommonPrefixLevelIndex(key);
			if (level_index == NULL_KEY) {
				return lowest_level.end();
			}

			size_t prefix_key = key >> (bit_count - 1 - level_index);
			auto ancestor = upper_levels[level_index].find(prefix_key);

          size_t desc_key = ancestor->second;
          while(desc_key == NULL_KEY) {
             size_t next_bit = (key >> (bit_count - 1 - (level_index + 1))) & 1;
             size_t child_prefix = (ancestor->first << 1) + next_bit;

             level_index++;
             ancestor = upper_levels[level_index].find(child_prefix);
             desc_key = ancestor->second;
          }

          auto desc_node = lowest_level.find(desc_key);

          if (key < desc_key) {
             if(desc_node == lowest_level.begin()) return lowest_level.end();
             return std::prev(desc_node);
          }
          return desc_node;
       }

		const_iter_lowest_level predecessorInternal(const size_t& key) const {
          const_iter_lowest_level node = lowest_level.find(key);
          if(node != lowest_level.cend()){
             if (node == lowest_level.cbegin()) return lowest_level.cend();
             return std::prev(node);
          }

          size_t level_index = findLongestCommonPrefixLevelIndex(key);
			if (level_index == NULL_KEY) {
				return lowest_level.cend();
			}

			size_t prefix_key = key >> (bit_count - 1 - level_index);
			const_iter_upper_levels ancestor = upper_levels[level_index].find(prefix_key);

          size_t desc_key = ancestor->second;
          while(desc_key == NULL_KEY) {
             size_t next_bit = (key >> (bit_count - 1 - (level_index + 1))) & 1;
             size_t child_prefix = (ancestor->first << 1) + next_bit;

             level_index++;
             ancestor = upper_levels[level_index].find(child_prefix);
             desc_key = ancestor->second;
          }

          const_iter_lowest_level desc_node = lowest_level.find(desc_key);

          if (key < desc_key) {
             if(desc_node == lowest_level.begin()) return lowest_level.end();
             return std::prev(desc_node);
          }
          return desc_node;
       }

		iter_lowest_level successorInternal(const size_t& key) {
          iter_lowest_level node = lowest_level.find(key);
          if (node != lowest_level.end()) {
              iter_lowest_level next_node = std::next(node);
              return next_node;
          }

			size_t level_index = findLongestCommonPrefixLevelIndex(key);
			if (level_index == NULL_KEY) {
				return lowest_level.begin();
			}

			size_t prefix_key = key >> (bit_count - 1 - level_index);
			iter_upper_levels ancestor = upper_levels[level_index].find(prefix_key);

          size_t desc_key = ancestor->second;
          while(desc_key == NULL_KEY) {
             size_t next_bit = (key >> (bit_count - 1 - (level_index + 1))) & 1;
             size_t child_prefix = (ancestor->first << 1) + next_bit;

             level_index++;
             ancestor = upper_levels[level_index].find(child_prefix);
             desc_key = ancestor->second;
          }

          iter_lowest_level desc_node = lowest_level.find(desc_key);

          if(key > desc_key) {
             iter_lowest_level next_node = std::next(desc_node);
             return next_node;
          }
          return desc_node;
       }

		const_iter_lowest_level successorInternal(const size_t& key) const {
          const_iter_lowest_level node = lowest_level.find(key);
          if (node != lowest_level.cend()) {
              const_iter_lowest_level next_node = std::next(node);
              return next_node;
          }

			size_t level_index = findLongestCommonPrefixLevelIndex(key);
			if (level_index == NULL_KEY) {
				return lowest_level.cbegin();
			}

			size_t prefix_key = key >> (bit_count - 1 - level_index);
			const_iter_upper_levels ancestor = upper_levels[level_index].find(prefix_key);

          size_t desc_key = ancestor->second;
          while(desc_key == NULL_KEY) {
             size_t next_bit = (key >> (bit_count - 1 - (level_index + 1))) & 1;
             size_t child_prefix = (ancestor->first << 1) + next_bit;

             level_index++;
             ancestor = upper_levels[level_index].find(child_prefix);
             desc_key = ancestor->second;
          }

          const_iter_lowest_level desc_node = lowest_level.find(desc_key);

          if(key > desc_key) {
             const_iter_lowest_level next_node = std::next(desc_node);
             return next_node;
          }
          return desc_node;
       }

	public:
		// Public predecessor/successor that work with Key
		iter_lowest_level predecessor(const Key& key) {
			return predecessorInternal(key2Internal(key));
		}

		const_iter_lowest_level predecessor(const Key& key) const {
			return predecessorInternal(key2Internal(key));
		}

		iter_lowest_level successor(const Key& key) {
			return successorInternal(key2Internal(key));
		}

		const_iter_lowest_level successor(const Key& key) const {
			return successorInternal(key2Internal(key));
		}

        size_t findLongestCommonPrefixLevelIndex(const size_t& key){
			size_t target_prefix_level_i = NULL_KEY;
            size_t low = 0;
            size_t high = bit_count-2;
            while(low <= high){
                size_t midpoint = low + (high - low) / 2;
                size_t curr_prefix = key >> (bit_count - (midpoint + 1));
            	if(upper_levels[midpoint].find(curr_prefix) != upper_levels[midpoint].end()){
                    low = midpoint + 1;
                    target_prefix_level_i = midpoint;
                }
                else {
                    if(high == 0){ break; }
                    high = midpoint - 1;
                }
            }
            return target_prefix_level_i;
        }

		size_t findLongestCommonPrefixLevelIndex(const size_t& key) const{
			size_t target_prefix_level_i = NULL_KEY;
            size_t low = 0;
            size_t high = bit_count-2;
            while(low <= high){
                const size_t midpoint = low + (high - low) / 2;
                size_t curr_prefix = key >> (bit_count - (midpoint + 1));
            	if(upper_levels[midpoint].find(curr_prefix) != upper_levels[midpoint].end()){
                    low = midpoint + 1;
                    target_prefix_level_i = midpoint;
                }
                else {
                    if(high == 0){ break; }
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
