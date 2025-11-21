#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include "RandomDatasetGenerator.h"
#include <SFML/Graphics.hpp>

//fastest single-threaded map candidates for all int types
#include "Radix_Flat_Map.h" //should be fast with read-heavy workloads
//#include "Batch_N_Hash_List.h" //should be fast with write-heavy workloads or batch lookup only - O(1) find and delete
#include "Batch_List.h" //should be fast with write-heavy workloads or batch lookup only - O(N) find and delete
#include <map> //should theoretically be fast for insertion and deletion
#include "X-fast_Trie.h" //theoretically the fastest
#include "AVL_Tree.h" //should theoretically be fast for lookup, successor, and predecessor
#include "Hash_Map_AVL_Tree.h" //should theoretically be faster than both red-black tree and AVL tree
#include "Treap.h" //should have lowest constant factors

sf::VertexArray createPlot(const std::vector<sf::Vector2f>& points, float max_x, float max_y,
						   const sf::Color& color, float plot_width, float height, float padding, float x_offset) {
	sf::VertexArray plot(sf::LinesStrip);
	float plotWidth = plot_width - 2 * padding;
	float plotHeight = height - 2 * padding;

	for (const auto& p : points) {
		// Scale and shift coordinaates to fit the plot area
		float x = x_offset + padding + (p.x / max_x) * plotWidth;
		float y = padding + plotHeight - (p.y / max_y) * plotHeight; // Y is inverted
		plot.append(sf::Vertex(sf::Vector2f(x, y), color));
	}
	return plot;
}

constexpr size_t Xpoint_MAX = 100;
constexpr size_t STRIDE = Xpoint_MAX/20;

enum class QueryType {
	INSERT,
	FIND,
	SUCCESSOR,
	PREDECESSOR,
	ERASE,
	RANGE
};

std::string queryTypeToString(QueryType type) {
	switch(type) {
		case QueryType::INSERT: return "Insert";
		case QueryType::FIND: return "Find";
		case QueryType::SUCCESSOR: return "Successor";
		case QueryType::PREDECESSOR: return "Predecessor";
		case QueryType::ERASE: return "Erase";
		case QueryType::RANGE: return "Range";
	}
	return "Unknown";
}

void runBenchmarks(QueryType queryType,
				   std::vector<sf::Vector2f>& stl_points,
				   std::vector<sf::Vector2f>& rf_points,
				   std::vector<sf::Vector2f>& rf_batch_points,
				   std::vector<sf::Vector2f>& batch_list_points,
				   std::vector<sf::Vector2f>& list_points,
				   //std::vector<sf::Vector2f>& bnhl_points,
				   //std::vector<sf::Vector2f>& hash_list_points,
				   std::vector<sf::Vector2f>& xft_points,
				   std::vector<sf::Vector2f>& avl_points,
				   std::vector<sf::Vector2f>& hash_avl_points,
				   std::vector<sf::Vector2f>& treap_points) {

	// Clear previous data
	stl_points.clear();
	rf_points.clear();
	rf_batch_points.clear();
	batch_list_points.clear();
	list_points.clear();
	//bnhl_points.clear();
	//hash_list_points.clear();
	xft_points.clear();
	avl_points.clear();
	hash_avl_points.clear();
	treap_points.clear();

	for(size_t i = 0; i <= Xpoint_MAX; i += STRIDE) {
		// Init maps
		std::map<size_t,int> stl_map;
		Radix_Flat_Map<size_t,int> rf_map;
		rf_map.reserve(i);
		Radix_Flat_Map<size_t,int> rf_batch_map;
		rf_batch_map.reserve(i);
		Batch_List<size_t,int> batch_list;
		Batch_List<size_t,int> list;
		//Batch_N_Hash_List<size_t,int> bnhl(i);
		//Batch_N_Hash_List<size_t,int> hash_list(i);
		XFastTrie<size_t,int> xft(i);
		AVL_Tree<size_t,int> avl_tree;
		Hash_Map_AVL_Tree<size_t,int> hash_avl_tree(i);
		Treap<size_t,int> treap;

		// Init dataset
		RandomDatasetGenerator rand_dataset(i);

		// Insert all data into the maps, except for insertion query type
		if (queryType != QueryType::INSERT) {
			for(size_t j = 0; j < i; j++) {
				stl_map.emplace(rand_dataset.random_size_ts[j], rand_dataset.random_ints[j]);
				//bnhl.addHead(rand_dataset.random_size_ts[j], rand_dataset.random_ints[j]);
				//hash_list.addHead(rand_dataset.random_size_ts[j], rand_dataset.random_ints[j]);
				xft.insert(rand_dataset.random_size_ts[j], rand_dataset.random_ints[j]);
				avl_tree.insert(rand_dataset.random_size_ts[j], rand_dataset.random_ints[j]);
				hash_avl_tree.insert(rand_dataset.random_size_ts[j], rand_dataset.random_ints[j]);
				treap.insert(rand_dataset.random_size_ts[j], rand_dataset.random_ints[j]);
			}
			// For batch map, use batch insert
			std::vector<std::pair<size_t, int>> batch_data;
			batch_data.reserve(i);
			for(size_t j = 0; j < i; j++) {
				batch_data.emplace_back(rand_dataset.random_size_ts[j], rand_dataset.random_ints[j]);
			}
			batch_list.batch_insert(batch_data.begin(), batch_data.end());
			list.batch_insert(batch_data.begin(), batch_data.end());
			rf_batch_map.insert_batch(batch_data.begin(), batch_data.end());
			rf_map.insert_batch(batch_data.begin(), batch_data.end());
		}

		// Create a new dataset with different seed for queries
		RandomDatasetGenerator query_dataset(i + 12345);

		auto start = std::chrono::high_resolution_clock::now();
		auto elapsed = 0.0;

		// Perform the selected query type
		switch(queryType) {
			case QueryType::INSERT: {
				// standard library map (red)
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					stl_map.emplace(rand_dataset.random_size_ts[j],rand_dataset.random_ints[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				stl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// radix flat map (green)
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					rf_map.insert(rand_dataset.random_size_ts[j],rand_dataset.random_ints[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				rf_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// radix flat map BATCH insertion (orange)
				std::vector<std::pair<size_t, int>> batch_data;
				batch_data.reserve(i);
				for(size_t j = 0; j < i; j++) {
					batch_data.emplace_back(rand_dataset.random_size_ts[j], rand_dataset.random_ints[j]);
				}
				start = std::chrono::high_resolution_clock::now();
				rf_batch_map.insert_batch(batch_data.begin(), batch_data.end());
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				rf_batch_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// batch list (cyan)
				start = std::chrono::high_resolution_clock::now();
				batch_list.batch_insert(batch_data.begin(), batch_data.end());
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				batch_list_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// list (light grey)
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					list.insert(rand_dataset.random_size_ts[j],rand_dataset.random_ints[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				list_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				/*
				// batch N Hash list (cyan)
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					bnhl.addHead(rand_dataset.random_size_ts[j],rand_dataset.random_ints[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				bnhl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// hash list (light grey)
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					hash_list.addHead(rand_dataset.random_size_ts[j],rand_dataset.random_ints[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				hash_list_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				*/
				// X-fast trie (yellow)
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					xft.insert(rand_dataset.random_size_ts[j],rand_dataset.random_ints[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				xft_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// AVL tree
				start = std::chrono::high_resolution_clock::now();
				for (size_t j = 0; j < i; j++) {
					avl_tree.insert(rand_dataset.random_size_ts[j], rand_dataset.random_ints[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				avl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// Hash map AVL tree
				start = std::chrono::high_resolution_clock::now();
				for (size_t j = 0; j < i; j++) {
					hash_avl_tree.insert(rand_dataset.random_size_ts[j], rand_dataset.random_ints[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				hash_avl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// Treap
				start = std::chrono::high_resolution_clock::now();
				for (size_t j = 0; j < i; j++) {
					treap.insert(rand_dataset.random_size_ts[j], rand_dataset.random_ints[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				treap_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));
				break;
			}

			case QueryType::FIND: {
				// standard library map
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					stl_map.find(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				stl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// radix flat map
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					rf_map.find(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				rf_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// radix flat map batch
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					rf_batch_map.find(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				rf_batch_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// batch list
				start = std::chrono::high_resolution_clock::now();
				batch_list.batch_find(query_dataset.random_size_ts);
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				batch_list_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// list
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					list.find(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				list_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				/*
				// batch N Hash list
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					bnhl.find(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				bnhl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// hash list
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					hash_list.find(query_dataset.random_size_ts[j]);
				}

				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				hash_list_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));
				*/

				// X-fast trie
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					xft.find(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				xft_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// AVL tree
				start = std::chrono::high_resolution_clock::now();
				for (size_t j = 0; j < i; j++) {
					avl_tree.find(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				avl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// Hash map AVL tree
				start = std::chrono::high_resolution_clock::now();
				for (size_t j = 0; j < i; j++) {
					hash_avl_tree.find(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				hash_avl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// Treap
				start = std::chrono::high_resolution_clock::now();
				for (size_t j = 0; j < i; j++) {
					treap.find(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				treap_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));
				break;
			}

			case QueryType::SUCCESSOR: {
				// standard library map
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					stl_map.upper_bound(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				stl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// radix flat map
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					rf_map.successor(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				rf_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// radix flat map batch
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					rf_batch_map.successor(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				rf_batch_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// batch list (uses batch operation)
				start = std::chrono::high_resolution_clock::now();
				batch_list.batch_successors(query_dataset.random_size_ts);
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				batch_list_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// list (uses individual queries)
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					list.successor(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				list_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				/*
				// batch N Hash list (uses batch operation)
				start = std::chrono::high_resolution_clock::now();
				bnhl.batch_successors(query_dataset.random_size_ts);
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				bnhl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// hash list (uses individual queries)
				start = std::chrono::high_resolution_clock::now();
				hash_list.successor(query_dataset.random_size_ts[0]);
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				hash_list_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed*(double)(i)));
				*/

				// X-fast trie
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					xft.successor(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				xft_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// AVL tree
				start = std::chrono::high_resolution_clock::now();
				for (size_t j = 0; j < i; j++) {
					avl_tree.upper_bound(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				avl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// Hash map AVL tree
				start = std::chrono::high_resolution_clock::now();
				for (size_t j = 0; j < i; j++) {
					hash_avl_tree.upper_bound(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				hash_avl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// Treap
				start = std::chrono::high_resolution_clock::now();
				for (size_t j = 0; j < i; j++) {
					treap.upper_bound(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				treap_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));
				break;
			}

			case QueryType::PREDECESSOR: {
				// standard library map
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					stl_map.lower_bound(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				stl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// radix flat map
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					rf_map.predecessor(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				rf_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// radix flat map batch
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					rf_batch_map.predecessor(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				rf_batch_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// batch list (uses batch operation)
				start = std::chrono::high_resolution_clock::now();
				batch_list.batch_predecessors(query_dataset.random_size_ts);
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				batch_list_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// list (uses individual queries)
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					list.predecessor(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				list_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				/*
				// batch N Hash list (uses batch operation)
				start = std::chrono::high_resolution_clock::now();
				bnhl.batch_predecessors(query_dataset.random_size_ts);
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				bnhl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// hash list (uses individual queries)
				start = std::chrono::high_resolution_clock::now();
				hash_list.predecessor(query_dataset.random_size_ts[0]);
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				hash_list_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed*(double)(i)));
				*/

				// X-fast trie
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					xft.predecessor(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				xft_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// AVL tree
				start = std::chrono::high_resolution_clock::now();
				for (size_t j = 0; j < i; j++) {
					avl_tree.lower_bound(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				avl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// Hash map AVL tree
				start = std::chrono::high_resolution_clock::now();
				for (size_t j = 0; j < i; j++) {
					hash_avl_tree.lower_bound(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				hash_avl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// Treap
				start = std::chrono::high_resolution_clock::now();
				for (size_t j = 0; j < i; j++) {
					treap.lower_bound(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				treap_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));
				break;
			}

			case QueryType::ERASE: {
				// standard library map
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					stl_map.erase(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				stl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// radix flat map
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					rf_map.erase(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				rf_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// radix flat map batch
				start = std::chrono::high_resolution_clock::now();
				rf_batch_map.erase_batch(query_dataset.random_size_ts.begin(),query_dataset.random_size_ts.end());
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				rf_batch_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// batch list
				start = std::chrono::high_resolution_clock::now();
				batch_list.batch_erase(query_dataset.random_size_ts);
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				batch_list_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// list
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					list.erase_key(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				list_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));
				/*
				// batch N Hash list
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					bnhl.remove(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				bnhl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// hash list
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					hash_list.remove(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				hash_list_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));
				*/

				// X-fast trie
				start = std::chrono::high_resolution_clock::now();
				for(size_t j = 0; j < i; j++) {
					xft.erase(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				xft_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// AVL tree
				start = std::chrono::high_resolution_clock::now();
				for (size_t j = 0; j < i; j++) {
					avl_tree.erase(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				avl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// Hash map AVL tree
				start = std::chrono::high_resolution_clock::now();
				for (size_t j = 0; j < i; j++) {
					hash_avl_tree.erase(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				hash_avl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// Treap
				start = std::chrono::high_resolution_clock::now();
				for (size_t j = 0; j < i; j++) {
					treap.erase(query_dataset.random_size_ts[j]);
				}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				treap_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));
				break;
			}

			case QueryType::RANGE: {
				// standard library map
				auto stl_end = stl_map.begin();
				for(size_t j = 0; j < i; j++) {
					++stl_end;
				}
				start = std::chrono::high_resolution_clock::now();
				for(auto iter = stl_map.begin(); iter != stl_end; ++iter) {}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				stl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// radix flat map
				auto rf_end = rf_map.begin() + i;
				start = std::chrono::high_resolution_clock::now();
				for(auto iter = rf_map.begin(); iter != rf_end; ++iter) {}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				rf_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// radix flat map batch
				auto rf_batch_end = rf_batch_map.begin() + i;
				start = std::chrono::high_resolution_clock::now();
				for(auto iter = rf_batch_map.begin(); iter != rf_batch_end; ++iter) {}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				rf_batch_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// batch list
				auto batch_list_end = batch_list.begin();
				for(size_t j = 0; j < i; j++) {
					++batch_list_end;
				}
				start = std::chrono::high_resolution_clock::now();
				for(auto iter = batch_list.begin(); iter != batch_list_end; ++iter) {}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				batch_list_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// list
				auto list_end = list.begin();
				for(size_t j = 0; j < i; j++) {
					++list_end;
				}
				start = std::chrono::high_resolution_clock::now();
				for(auto iter = list.begin(); iter != list_end; ++iter) {}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				list_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				/*
				// batch N Hash list
				auto hl_batch_end = bnhl.begin();
				for(size_t j = 0; j < i; j++) {
					++hl_batch_end;
				}
				start = std::chrono::high_resolution_clock::now();
				bnhl.sort_keys();
				for(auto iter = bnhl.begin(); iter != hl_batch_end; ++iter) {}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				bnhl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// hash list
				auto hl_end = hash_list.begin();
				for(size_t j = 0; j < i; j++) {
					++hl_end;
				}
				start = std::chrono::high_resolution_clock::now();
				hash_list.sort_keys();
				for(auto iter = hash_list.begin(); iter != hl_end; ++iter) {}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				hash_list_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));
				*/

				// X-fast trie
				auto xft_end = xft.begin();
				for(size_t j = 0; j < i; j++) {
					++xft_end;
				}
				start = std::chrono::high_resolution_clock::now();
				for(auto iter = xft.begin(); iter != xft_end; ++iter) {}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				xft_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// AVL tree
				auto avlt_end = avl_tree.begin();
				for(size_t j = 0; j < i; j++) {
					++avlt_end;
				}
				start = std::chrono::high_resolution_clock::now();
				for(auto iter = avl_tree.begin(); iter != avlt_end; ++iter) {}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				avl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// Hash map AVL tree
				auto hmavlt_end = hash_avl_tree.begin();
				for(size_t j = 0; j < i; j++) {
					++hmavlt_end;
				}
				start = std::chrono::high_resolution_clock::now();
				for(auto iter = hash_avl_tree.begin(); iter != hmavlt_end; ++iter) {}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				hash_avl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));

				// Treap
				auto treap_end = treap.begin();
				for(size_t j = 0; j < i; j++) {
					++treap_end;
				}
				start = std::chrono::high_resolution_clock::now();
				for(auto iter = treap.begin(); iter != treap_end; ++iter) {}
				elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
				treap_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));
				break;
			}
		}
	}
}

int main() {
	// Create separate point vectors for each data structure and query type
	std::map<QueryType, std::vector<sf::Vector2f>> stl_results;
	std::map<QueryType, std::vector<sf::Vector2f>> rf_results;
	std::map<QueryType, std::vector<sf::Vector2f>> rf_batch_results;
	std::map<QueryType, std::vector<sf::Vector2f>> batch_list_results;
	std::map<QueryType, std::vector<sf::Vector2f>> list_results;
	//std::map<QueryType, std::vector<sf::Vector2f>> bnhl_results;
	//std::map<QueryType, std::vector<sf::Vector2f>> hash_list_results;
	std::map<QueryType, std::vector<sf::Vector2f>> xft_results;
	std::map<QueryType, std::vector<sf::Vector2f>> avl_results;
	std::map<QueryType, std::vector<sf::Vector2f>> hash_avl_results;
	std::map<QueryType, std::vector<sf::Vector2f>> treap_results;

	// Run all benchmarks at startup
	std::vector<QueryType> all_query_types = {
		QueryType::INSERT, QueryType::FIND, QueryType::SUCCESSOR,
		QueryType::PREDECESSOR, QueryType::ERASE, QueryType::RANGE
	};

	std::cout << "Running benchmarks for all query types..." << std::endl;
	for (const auto& queryType : all_query_types) {
		std::cout << "  - " << queryTypeToString(queryType) << std::endl;
		runBenchmarks(queryType,
					  stl_results[queryType],
					  rf_results[queryType],
					  rf_batch_results[queryType],
					  batch_list_results[queryType],
					  list_results[queryType],
					  //bnhl_results[queryType],
					  //hash_list_results[queryType],
					  xft_results[queryType],
					  avl_results[queryType],
					  hash_avl_results[queryType],
					  treap_results[queryType]);
	}
	std::cout << "All benchmarks complete!" << std::endl;

	std::map<QueryType, bool> selectedQueries;
	for (const auto& qt : all_query_types) {
		selectedQueries[qt] = (qt == QueryType::INSERT);
	}

	std::vector<sf::Vector2f> stl_points;
	std::vector<sf::Vector2f> rf_points;
	std::vector<sf::Vector2f> rf_batch_points;
	std::vector<sf::Vector2f> batch_list_points;
	std::vector<sf::Vector2f> list_points;
	//std::vector<sf::Vector2f> bnhl_points;
	//std::vector<sf::Vector2f> hash_list_points;
	std::vector<sf::Vector2f> xft_points;
	std::vector<sf::Vector2f> avl_points;
	std::vector<sf::Vector2f> hash_avl_points;
	std::vector<sf::Vector2f> treap_points;

	auto recalculateCombinedTimes = [&]() {
		stl_points.clear();
		rf_points.clear();
		rf_batch_points.clear();
		batch_list_points.clear();
		list_points.clear();
		//bnhl_points.clear();
		//hash_list_points.clear();
		xft_points.clear();
		avl_points.clear();
		hash_avl_points.clear();
		treap_points.clear();

		size_t numPoints = stl_results[QueryType::INSERT].size();

		for (size_t i = 0; i < numPoints; ++i) {
			float stl_sum = 0, rf_sum = 0, rf_batch_sum = 0;
			float batch_list_sum = 0, list_sum = 0;
			//float bnhl_sum = 0, hash_list_sum = 0;
			float xft_sum = 0, avl_sum = 0, hash_avl_sum = 0, treap_sum = 0;
			float x_value = 0;

			for (const auto& qt : all_query_types) {
				if (selectedQueries[qt]) {
					stl_sum += stl_results[qt][i].y;
					rf_sum += rf_results[qt][i].y;
					rf_batch_sum += rf_batch_results[qt][i].y;
					batch_list_sum += batch_list_results[qt][i].y;
					list_sum += list_results[qt][i].y;
					//bnhl_sum += bnhl_results[qt][i].y;
					//hash_list_sum += hash_list_results[qt][i].y;
					xft_sum += xft_results[qt][i].y;
					avl_sum += avl_results[qt][i].y;
					hash_avl_sum += hash_avl_results[qt][i].y;
					treap_sum += treap_results[qt][i].y;
					x_value = stl_results[qt][i].x; // All have same x values
				}
			}

			stl_points.emplace_back(x_value, stl_sum);
			rf_points.emplace_back(x_value, rf_sum);
			rf_batch_points.emplace_back(x_value, rf_batch_sum);
			batch_list_points.emplace_back(x_value, batch_list_sum);
			list_points.emplace_back(x_value, list_sum);
			//bnhl_points.emplace_back(x_value, bnhl_sum);
			//hash_list_points.emplace_back(x_value, hash_list_sum);
			xft_points.emplace_back(x_value, xft_sum);
			avl_points.emplace_back(x_value, avl_sum);
			hash_avl_points.emplace_back(x_value, hash_avl_sum);
			treap_points.emplace_back(x_value, treap_sum);
		}
	};

	recalculateCombinedTimes(); // Initial calculation with just INSERT selected


	// SFML Plotting - Initial dimensions
	float WIN_WIDTH = 1080.0f;
	float UI_WIDTH = 350.0f;
	float PLOT_WIDTH = WIN_WIDTH - UI_WIDTH;
	float WIN_HEIGHT = PLOT_WIDTH;
	constexpr float PADDING = 60.0f;

	sf::RenderWindow window(sf::VideoMode(static_cast<unsigned int>(WIN_WIDTH), static_cast<unsigned int>(WIN_HEIGHT)), "Map Insertion Benchmark", sf::Style::Default);
	window.setFramerateLimit(60);

	// Function to update dimensions when window is resized
	auto updateDimensions = [&]() {
		sf::Vector2u size = window.getSize();
		WIN_WIDTH = static_cast<float>(size.x);
		WIN_HEIGHT = static_cast<float>(size.y);
		// Keep UI_WIDTH proportional or fixed
		if (WIN_WIDTH < 800) {
			UI_WIDTH = WIN_WIDTH * 0.3f; // Proportional for small windows
		} else {
			UI_WIDTH = 350.0f; // Fixed for normal/large windows
		}
		PLOT_WIDTH = WIN_WIDTH - UI_WIDTH;
	};

	// Track visibility and data for each plot
	struct PlotInfo {
		std::vector<sf::Vector2f>* points;
		sf::Color color;
		bool visible;
		std::string name;
		sf::VertexArray* vertexArray;
	};

	sf::VertexArray plot_stl;
	sf::VertexArray plot_rf;
	sf::VertexArray plot_rf_batch;
	sf::VertexArray plot_batch_list;
	sf::VertexArray plot_list;
	//sf::VertexArray plot_bnhl;
	//sf::VertexArray plot_hash_list;
	sf::VertexArray plot_xft;
	sf::VertexArray plot_avl;
	sf::VertexArray plot_hash_avl;
	sf::VertexArray plot_treap;

	std::vector<PlotInfo> plots = {
		{&stl_points, sf::Color::Red, true, "Red: std::map (red-black tree)", &plot_stl},
		{&rf_points, sf::Color::Green, true, "Green: Radix_Flat_Map (No batching)", &plot_rf},
		{&rf_batch_points, sf::Color(255, 165, 0), true, "Orange: Radix_Flat_Map (Batch = N)", &plot_rf_batch},
		{&batch_list_points, sf::Color::Cyan, true, "Cyan: Batch_List (Batch = N)", &plot_batch_list},
		{&list_points, sf::Color(180, 180, 180), true, "Light Grey: List (No batching)", &plot_list},
		//{&bnhl_points, sf::Color::Cyan, true, "Cyan: Batch_N_Hash_List (Batch = N)", &plot_bnhl},
		//{&hash_list_points, sf::Color(180, 180, 180), true, "Light Grey: Batch_N_Hash_List (No batching)", &plot_hash_list},
		{&xft_points, sf::Color::Yellow, true, "Yellow: XFastTrie", &plot_xft},
		{&avl_points, sf::Color(128, 128, 128), true, "Grey: AVL_Tree", &plot_avl},
		{&hash_avl_points, sf::Color::Magenta, true, "Magenta: Hash_Map_AVL_Tree", &plot_hash_avl},
		{&treap_points, sf::Color(255, 192, 203), true, "Pink: Treap", &plot_treap}
	};

	float max_x = static_cast<float>(Xpoint_MAX);

	// Function to regenerate all plots
	auto regeneratePlots = [&plots, &max_x, &PLOT_WIDTH, &WIN_HEIGHT, &PADDING, &UI_WIDTH]() {
		float max_y = 0.000001f;
		for (const auto& plot : plots) {
			if (plot.visible) {
				for (const auto& point : *plot.points) {
					if (point.y > max_y) {
						max_y = point.y;
					}
				}
			}
		}

		for (auto& plot : plots) {
			*plot.vertexArray = createPlot(*plot.points, max_x, max_y, plot.color, PLOT_WIDTH, WIN_HEIGHT, PADDING, UI_WIDTH);
		}

		return max_y;
	};

	float max_y = regeneratePlots();

	// Create axes (only in plot area on the right)
	sf::VertexArray axes(sf::Lines);
	// Y-Axis
	axes.append(sf::Vertex(sf::Vector2f(UI_WIDTH + PADDING, PADDING), sf::Color::White));
	axes.append(sf::Vertex(sf::Vector2f(UI_WIDTH + PADDING, WIN_HEIGHT - PADDING), sf::Color::White));
	// X-Axis
	axes.append(sf::Vertex(sf::Vector2f(UI_WIDTH + PADDING, WIN_HEIGHT - PADDING), sf::Color::White));
	axes.append(sf::Vertex(sf::Vector2f(WIN_WIDTH - PADDING, WIN_HEIGHT - PADDING), sf::Color::White));

	// Vertical separator line between UI and plot
	sf::VertexArray separator(sf::Lines);
	separator.append(sf::Vertex(sf::Vector2f(UI_WIDTH, 0), sf::Color(80, 80, 80)));
	separator.append(sf::Vertex(sf::Vector2f(UI_WIDTH, WIN_HEIGHT), sf::Color(80, 80, 80)));

	// Load Font
	sf::Font font;
	bool font_loaded = false;

	// Try default font paths depending on OS
	#ifdef _WIN32
		font_loaded = font.loadFromFile("C:/Windows/Fonts/Arial.ttf");
	#elif __APPLE__
		font_loaded = font.loadFromFile("/Library/Fonts/Arial.ttf");
	#else
		font_loaded = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
	#endif

	if (!font_loaded) {
		std::cerr << "Could not load a default font. Labels will be missing.\n";
	}

	// Create Labels
	sf::Text title, y_label, x_label, max_y_label, max_x_label;
	sf::Text legend_label;
	std::vector<sf::Text> legend_texts;
	std::vector<sf::CircleShape> legend_circles;

	if (font_loaded) {
		title.setFont(font);
		title.setCharacterSize(20);

		y_label.setFont(font);
		y_label.setString("Time (s)");
		y_label.setCharacterSize(16);
		y_label.setRotation(-90);
		y_label.setPosition(UI_WIDTH + PADDING / 2.5f, WIN_HEIGHT / 2.0f + y_label.getGlobalBounds().width / 2.0f);

		x_label.setFont(font);
		x_label.setString("N (unsigned int)");
		x_label.setCharacterSize(16);
		x_label.setPosition(UI_WIDTH + PLOT_WIDTH / 2.0f - x_label.getGlobalBounds().width / 2.0f, WIN_HEIGHT - PADDING / 1.5f);

		max_x_label.setFont(font);
		max_x_label.setString(std::to_string(Xpoint_MAX));
		max_x_label.setCharacterSize(12);
		max_x_label.setPosition(WIN_WIDTH - PADDING - max_x_label.getGlobalBounds().width - 5, WIN_HEIGHT - PADDING + 5);

		// Initialize max_y label
		std::stringstream ss;
		ss << std::fixed << std::setprecision(4) << max_y;
		max_y_label.setFont(font);
		max_y_label.setString(ss.str() + "s");
		max_y_label.setCharacterSize(12);
		max_y_label.setPosition(UI_WIDTH + PADDING + 5, PADDING - 5);

		// Legend label
		legend_label.setFont(font);
		legend_label.setString("Legend (Click circles to toggle lines):");
		legend_label.setCharacterSize(14);
		legend_label.setPosition(11.0f, 20.0f);
		legend_label.setFillColor(sf::Color::White);

		// Legend with circles (now in UI section on the left, shifted down)
		float legend_x = 5.0f;
		float legend_y = 44.0f;  // Shifted down from 20 to 44 to make room for label
		float circle_radius = 6.0f;

		for (size_t i = 0; i < plots.size(); ++i) {
			// Create circle
			sf::CircleShape circle(circle_radius);
			circle.setFillColor(plots[i].color);
			circle.setPosition(legend_x, legend_y + i * 20 + 3);
			legend_circles.push_back(circle);

			// Create text
			sf::Text text;
			text.setFont(font);
			text.setString(plots[i].name);
			text.setCharacterSize(12);
			text.setPosition(legend_x + circle_radius * 2 + 5, legend_y + i * 20);
			text.setFillColor(plots[i].color);
			legend_texts.push_back(text);
		}
	}

	// Query Type Selection Buttons (now in UI section on the left)
	std::vector<sf::RectangleShape> query_buttons;
	std::vector<sf::Text> query_button_texts;

	sf::Text query_selector_label;
	float selector_x = 11.0f;  // 11 pixels from left edge
	float selector_y = 44.0f + plots.size() * 20 + 38;  // Adjusted for shifted legend (was 20.0f)
	float button_spacing = 5.0f;
	// Calculate button width with 11px margins on each side
	float ui_section_width = UI_WIDTH - 22.0f;  // Minus 11px on each side
	float button_width = (ui_section_width - 2 * button_spacing) / 3.0f;  // 3 columns with 2 gaps
	float button_height = 25.0f;

	if (font_loaded) {
		query_selector_label.setFont(font);
		query_selector_label.setString("Query Types (click to toggle):");
		query_selector_label.setCharacterSize(14);
		query_selector_label.setPosition(selector_x, selector_y - 24);  // Increased from -20 to -24 for 4 more pixels
		query_selector_label.setFillColor(sf::Color::White);

		for (size_t i = 0; i < all_query_types.size(); ++i) {
			// Arrange buttons in 3x2 grid
			float x_offset = (i % 3) * (button_width + button_spacing);
			float y_offset = (i / 3) * (button_height + button_spacing);

			// Create button
			sf::RectangleShape button(sf::Vector2f(button_width, button_height));
			button.setPosition(selector_x + x_offset, selector_y + y_offset);
			button.setOutlineThickness(2.0f);

			// Highlight if selected
			if (selectedQueries[all_query_types[i]]) {
				button.setFillColor(sf::Color(50, 50, 150));
				button.setOutlineColor(sf::Color::White);
			} else {
				button.setFillColor(sf::Color(30, 30, 60));
				button.setOutlineColor(sf::Color(100, 100, 100));
			}

			query_buttons.push_back(button);

			// Create button text
			sf::Text button_text;
			button_text.setFont(font);
			button_text.setString(queryTypeToString(all_query_types[i]));
			button_text.setCharacterSize(11);
			button_text.setFillColor(sf::Color::White);

			// Center text in button
			sf::FloatRect text_bounds = button_text.getLocalBounds();
			button_text.setPosition(
				selector_x + x_offset + (button_width - text_bounds.width) / 2.0f - text_bounds.left,
				selector_y + y_offset + (button_height - text_bounds.height) / 2.0f - text_bounds.top - 2
			);

			query_button_texts.push_back(button_text);
		}
	}

	// Function to generate title
	auto generateTitle = [&]() -> std::string {
		return "Queries - N unsigned ints (Max N = " + std::to_string(Xpoint_MAX) + ")";
	};

	// Set initial title (centered over plot area on the right)
	if (font_loaded) {
		title.setString(generateTitle());
		title.setPosition(UI_WIDTH + PLOT_WIDTH / 2.0f - title.getGlobalBounds().width / 2.0f, PADDING / 4.0f);
	}

	// Create cursor coordinate texts
	sf::Text cursor_x_text, cursor_y_text;
	sf::Color light_blue(100, 180, 255);
	if (font_loaded) {
		cursor_x_text.setFont(font);
		cursor_x_text.setCharacterSize(14);
		cursor_x_text.setFillColor(light_blue);

		cursor_y_text.setFont(font);
		cursor_y_text.setCharacterSize(14);
		cursor_y_text.setFillColor(light_blue);
	}

	// Function to recalculate all UI element positions
	auto recalculateUIPositions = [&]() {
		// Update separator
		separator.clear();
		separator.append(sf::Vertex(sf::Vector2f(UI_WIDTH, 0), sf::Color(80, 80, 80)));
		separator.append(sf::Vertex(sf::Vector2f(UI_WIDTH, WIN_HEIGHT), sf::Color(80, 80, 80)));

		// Update axes
		axes.clear();
		axes.append(sf::Vertex(sf::Vector2f(UI_WIDTH + PADDING, PADDING), sf::Color::White));
		axes.append(sf::Vertex(sf::Vector2f(UI_WIDTH + PADDING, WIN_HEIGHT - PADDING), sf::Color::White));
		axes.append(sf::Vertex(sf::Vector2f(UI_WIDTH + PADDING, WIN_HEIGHT - PADDING), sf::Color::White));
		axes.append(sf::Vertex(sf::Vector2f(WIN_WIDTH - PADDING, WIN_HEIGHT - PADDING), sf::Color::White));

		if (font_loaded) {
			// Update labels
			y_label.setPosition(UI_WIDTH + PADDING / 2.5f, WIN_HEIGHT / 2.0f + y_label.getGlobalBounds().width / 2.0f);
			x_label.setPosition(UI_WIDTH + PLOT_WIDTH / 2.0f - x_label.getGlobalBounds().width / 2.0f, WIN_HEIGHT - PADDING / 1.5f);
			max_x_label.setPosition(WIN_WIDTH - PADDING - max_x_label.getGlobalBounds().width - 5, WIN_HEIGHT - PADDING + 5);
			max_y_label.setPosition(UI_WIDTH + PADDING + 5, PADDING - 5);
			title.setPosition(UI_WIDTH + PLOT_WIDTH / 2.0f - title.getGlobalBounds().width / 2.0f, PADDING / 4.0f);

			// Update legend circles and texts
			float legend_x = 5.0f;
			float legend_y = 44.0f;
			float circle_radius = 6.0f;
			for (size_t i = 0; i < legend_circles.size(); ++i) {
				legend_circles[i].setPosition(legend_x, legend_y + i * 20 + 3);
				legend_texts[i].setPosition(legend_x + circle_radius * 2 + 5, legend_y + i * 20);
			}

			// Update query buttons and texts
			float selector_x = 11.0f;
			float selector_y = 44.0f + plots.size() * 20 + 38;
			float button_spacing = 5.0f;
			float ui_section_width = UI_WIDTH - 22.0f;
			float button_width = (ui_section_width - 2 * button_spacing) / 3.0f;
			float button_height = 25.0f;

			query_selector_label.setPosition(selector_x, selector_y - 24);

			for (size_t i = 0; i < query_buttons.size(); ++i) {
				float x_offset = (i % 3) * (button_width + button_spacing);
				float y_offset = (i / 3) * (button_height + button_spacing);

				query_buttons[i].setPosition(selector_x + x_offset, selector_y + y_offset);
				query_buttons[i].setSize(sf::Vector2f(button_width, button_height));

				// Recalculate text centering
				sf::FloatRect text_bounds = query_button_texts[i].getLocalBounds();
				query_button_texts[i].setPosition(
					selector_x + x_offset + (button_width - text_bounds.width) / 2.0f - text_bounds.left,
					selector_y + y_offset + (button_height - text_bounds.height) / 2.0f - text_bounds.top - 2
				);
			}
		}

		// Regenerate plots with new dimensions
		max_y = regeneratePlots();
	};

	// Main SFML Loop
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::Resized) {
				sf::FloatRect visibleArea(0, 0, static_cast<float>(event.size.width), static_cast<float>(event.size.height));
				window.setView(sf::View(visibleArea));

				updateDimensions();
				recalculateUIPositions();
			}

			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::F11) {
					// Toggle fullscreen
					static bool isFullscreen = false;
					if (!isFullscreen) {
						window.create(sf::VideoMode::getDesktopMode(), "Map Insertion Benchmark", sf::Style::Fullscreen);
						window.setFramerateLimit(60);
						isFullscreen = true;
					} else {
						window.create(sf::VideoMode(1080, 730), "Map Insertion Benchmark", sf::Style::Default);
						window.setFramerateLimit(60);
						isFullscreen = false;
					}
					updateDimensions();
					recalculateUIPositions();
				}
			}

			if (event.type == sf::Event::MouseButtonPressed) {
				if (event.mouseButton.button == sf::Mouse::Left) {
					sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x),
										 static_cast<float>(event.mouseButton.y));

					// Check if any circle was clicked
					bool circle_clicked = false;
					for (size_t i = 0; i < legend_circles.size(); ++i) {
						if (legend_circles[i].getGlobalBounds().contains(mousePos)) {
							// Toggle visibility
							plots[i].visible = !plots[i].visible;

							// Update circle appearance
							if (plots[i].visible) {
								legend_circles[i].setFillColor(plots[i].color);
							} else {
								// Darken the color
								sf::Color darkColor = plots[i].color;
								darkColor.r /= 3;
								darkColor.g /= 3;
								darkColor.b /= 3;
								legend_circles[i].setFillColor(darkColor);
							}

							// Regenerate plots with new scale
							max_y = regeneratePlots();

							// Update max_y label
							if (font_loaded) {
								std::stringstream ss;
								ss << std::fixed << std::setprecision(4) << max_y;
								max_y_label.setFont(font);
								max_y_label.setString(ss.str() + "s");
								max_y_label.setCharacterSize(12);
								max_y_label.setPosition(UI_WIDTH + PADDING + 5, PADDING - 5);
							}

							circle_clicked = true;
							break;
						}
					}

					// Check if any query button was clicked
					if (!circle_clicked) {
						for (size_t i = 0; i < query_buttons.size(); ++i) {
							if (query_buttons[i].getGlobalBounds().contains(mousePos)) {
								// Toggle query selection
								selectedQueries[all_query_types[i]] = !selectedQueries[all_query_types[i]];

								// Update button appearance
								if (selectedQueries[all_query_types[i]]) {
									query_buttons[i].setFillColor(sf::Color(50, 50, 150));
									query_buttons[i].setOutlineColor(sf::Color::White);
								} else {
									query_buttons[i].setFillColor(sf::Color(30, 30, 60));
									query_buttons[i].setOutlineColor(sf::Color(100, 100, 100));
								}

								recalculateCombinedTimes();

								max_y = regeneratePlots();

								// Update max_y label
								if (font_loaded) {
									std::stringstream ss;
									ss << std::fixed << std::setprecision(4) << max_y;
									max_y_label.setString(ss.str() + "s");
									max_y_label.setPosition(UI_WIDTH + PADDING + 5, PADDING - 5);
								}

								break;
							}
						}
					}
				}
			}
		}

		// Check for mouse hover over buttons for visual feedback
		sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
		sf::Vector2f mousePos(static_cast<float>(mousePixelPos.x), static_cast<float>(mousePixelPos.y));

		for (size_t i = 0; i < query_buttons.size(); ++i) {
			if (query_buttons[i].getGlobalBounds().contains(mousePos)) {
				// Brighten button on hover if not selected
				if (!selectedQueries[all_query_types[i]]) {
					query_buttons[i].setFillColor(sf::Color(40, 40, 80));
				}
			} else {
				// Reset to default color
				if (!selectedQueries[all_query_types[i]]) {
					query_buttons[i].setFillColor(sf::Color(30, 30, 60));
				}
			}
		}

		// Create crosshair cursor lines and coordinate display
		bool cursor_in_plot_area = false;
		sf::VertexArray cursor_lines(sf::Lines);
		float plot_width = PLOT_WIDTH - 2 * PADDING;
		float plot_height = WIN_HEIGHT - 2 * PADDING;

		// Check if mouse is in the plot area on the right, and not in UI section on the left
		if (mousePos.x >= UI_WIDTH + PADDING && mousePos.x <= WIN_WIDTH - PADDING &&
			mousePos.y >= PADDING && mousePos.y <= WIN_HEIGHT - PADDING) {
			cursor_in_plot_area = true;

			// Calculate data coordinates from screen coordinates
			float data_x = ((mousePos.x - UI_WIDTH - PADDING) / plot_width) * max_x;
			float data_y = max_y - ((mousePos.y - PADDING) / plot_height) * max_y;

			// Create dashed vertical and horizontal lines (cross-hair)
			float dash_length = 8.0f;
			float gap_length = 4.0f;
			float segment_length = dash_length + gap_length;

			// Vertical dashed line
			for (float y = PADDING; y < WIN_HEIGHT - PADDING; y += segment_length) {
				float end_y = std::min(y + dash_length, WIN_HEIGHT - PADDING);
				cursor_lines.append(sf::Vertex(sf::Vector2f(mousePos.x, y), light_blue));
				cursor_lines.append(sf::Vertex(sf::Vector2f(mousePos.x, end_y), light_blue));
			}

			// Horizontal dashed line (only in plot area)
			for (float x = UI_WIDTH + PADDING; x < WIN_WIDTH - PADDING; x += segment_length) {
				float end_x = std::min(x + dash_length, WIN_WIDTH - PADDING);
				cursor_lines.append(sf::Vertex(sf::Vector2f(x, mousePos.y), light_blue));
				cursor_lines.append(sf::Vertex(sf::Vector2f(end_x, mousePos.y), light_blue));
			}

			// Update coordinate text displays
			if (font_loaded) {
				// X coordinate text
				std::stringstream x_ss;
				x_ss << std::fixed << std::setprecision(1) << data_x;
				cursor_x_text.setString(x_ss.str());

				// Y coordinate text
				std::stringstream y_ss;
				y_ss << std::fixed << std::setprecision(6) << data_y;
				cursor_y_text.setString(y_ss.str());

				// Position both texts near the cursor with offset
				float text_offset_x = 15.0f;
				float text_offset_y = 15.0f;

				// Get text dimensions for boundary checking
				float x_text_width = cursor_x_text.getGlobalBounds().width;
				float y_text_width = cursor_y_text.getGlobalBounds().width;
				float text_height = cursor_x_text.getGlobalBounds().height;

				// Position X text
				float x_text_x = mousePos.x + text_offset_x;
				float x_text_y = mousePos.y + text_offset_y;

				// Keep X text within plot boundaries
				if (x_text_x + x_text_width > WIN_WIDTH - PADDING - 5.0f) {
					x_text_x = mousePos.x - text_offset_x - x_text_width;
				}
				if (x_text_y + text_height > WIN_HEIGHT - PADDING - 5.0f) {
					x_text_y = mousePos.y - text_offset_y - text_height;
				}
				cursor_x_text.setPosition(x_text_x, x_text_y);

				// Position Y text below X text
				float y_text_x = mousePos.x + text_offset_x;
				float y_text_y = mousePos.y + text_offset_y + text_height + 5.0f;

				// Keep Y text within plot boundaries
				if (y_text_x + y_text_width > WIN_WIDTH - PADDING - 5.0f) {
					y_text_x = mousePos.x - text_offset_x - y_text_width;
				}
				if (y_text_y + text_height > WIN_HEIGHT - PADDING - 5.0f) {
					y_text_y = mousePos.y - text_offset_y - text_height * 2 - 5.0f;
				}
				cursor_y_text.setPosition(y_text_x, y_text_y);
			}
		}

		window.clear(sf::Color(10, 10, 30)); // Dark blue background

		// Draw plots (only if visible)
		for (const auto& plot : plots) {
			if (plot.visible) {
				window.draw(*plot.vertexArray);
			}
		}

		// Draw UI
		window.draw(axes);
		window.draw(separator);
		if (font_loaded) {
			window.draw(title);
			window.draw(x_label);
			window.draw(y_label);
			window.draw(max_x_label);
			window.draw(max_y_label);

			// Draw legend label
			window.draw(legend_label);

			for (const auto& circle : legend_circles) {
				window.draw(circle);
			}
			for (const auto& text : legend_texts) {
				window.draw(text);
			}

			// Draw query type buttons
			window.draw(query_selector_label);
			for (const auto& button : query_buttons) {
				window.draw(button);
			}
			for (const auto& text : query_button_texts) {
				window.draw(text);
			}
		}

		// Draw cursor crosshair and coordinates (only if in plot area)
		if (cursor_in_plot_area) {
			window.draw(cursor_lines);
			if (font_loaded) {
				window.draw(cursor_x_text);
				window.draw(cursor_y_text);
			}
		}

		window.display();
	}

	return 0;
}