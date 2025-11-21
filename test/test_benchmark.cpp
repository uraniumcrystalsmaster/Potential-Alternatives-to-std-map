#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <random>


//fastest single-threaded map candidates for all int and float types
#include "Radix_Flat_Map.h" //fast with read-heavy workloads
#include "Batch_N_Hash_List.h" //fast with write-heavy workloads or batch lookup only
#include <map> //best for abstract data types
#include "X-fast_Trie.h" //best for mixed workloads
#include "AVL_Tree.h" //could be better for abstract data types

//#include "SmallTestDataset.h" // Contains no duplicates for easier debugging; has 100 truly-random size_t and strings
#include <Hash_Map_AVL_Tree.h>
#include <set>
#include <Treap.h>

#include "RandomDatasetGenerator.h" // Larger version
using namespace std; //todo: remove when finished

/*
// PROTOTYPE PERFORMANCE SECTION
TEST_CASE("[TRIES] Predecessor-Heavy Queries: old vs Trie method", "[tries][performance][predecessor]") {
	constexpr size_t n = 30000;
	constexpr size_t q = 3000;



	//KEY is size type and VALUE is integer
	map<size_t, int> map;
	XFastTrie<int> xft(n);

	for (size_t i = 0; i < n; i++) {
		size_t key = i * 2;
		int value = static_cast<int>(i);
		map[key] = value;
		xft.insert(key, value);
	}

	const auto old_start = chrono::high_resolution_clock::now();
	size_t old_sum = 0;
	for (size_t i = 0; i < q; i++) {
		const size_t query = i * 3 + 1;
		auto iter = map.upper_bound(query);
		if (iter != map.begin()) {
			--iter;
			old_sum += iter->second;
		}
	}

	const double old_elapsed = chrono::duration<double>(chrono::high_resolution_clock::now() - old_start).count();

	const auto trie_start = chrono::high_resolution_clock::now();
	size_t trie_sum = 0;
	for (size_t i = 0; i < q; i++) {
		size_t query = i * 3 + 1;
		auto it = xft.predecessor(query);
		if (it != xft.end()) trie_sum += it.value();
	}

	const double trie_elapsed = chrono::duration<double>(chrono::high_resolution_clock::now() - trie_start).count();


	cout << fixed << setprecision(3);
	cout << "[OLD PERFORMANCE: PREDECESSOR] " << old_elapsed << " seconds\n";
	cout << "[TRIE PERFORMANCE: PREDECESSOR] " << trie_elapsed << " seconds\n";
	REQUIRE(old_elapsed > trie_elapsed);
}

TEST_CASE("[TRIES] Successor-Heavy Queries: old vs Trie method", "[tries][performance][successor]") {
	constexpr size_t n = 30000;
	constexpr size_t q = 3000;

	map<size_t, int> map;
	XFastTrie<int> xft(n);

	for (size_t i = 0; i < n; i++) {
		size_t key = i * 2;
		int value = static_cast<int>(i);
		map[key] = value;
		xft.insert(key, value);
	}

	const auto old_start = chrono::high_resolution_clock::now();
	size_t old_sum = 0;
	for (size_t i = 0; i < q; i++) {
		const size_t query = i * 3 + 1;
		size_t best = std::numeric_limits<size_t>::max();
		bool found = false;
		for (const auto &kv : map) {
			size_t k = kv.first;
			if (k >= query) {
				if (!found || k < best) best = k;
				found = true;
			}
		}
		if (found) old_sum += map[best];
	}
	const double old_elapsed = chrono::duration<double>(chrono::high_resolution_clock::now() - old_start).count();

	const auto trie_start = chrono::high_resolution_clock::now();
	size_t trie_sum = 0;
	for (size_t i = 0; i < q; i++) {
		size_t query = i * 3 + 1;
		auto it = xft.successor(query);
		if (it != xft.end()) trie_sum += it.value();
	}
	const double trie_elapsed = chrono::duration<double>(chrono::high_resolution_clock::now() - trie_start).count();

	cout << fixed << setprecision(3);
	cout << "[OLD PERFORMANCE: SUCCESSOR] " << old_elapsed << " seconds\n";
	cout << "[TRIE PERFORMANCE: SUCCESSOR] " << trie_elapsed << " seconds\n";
	REQUIRE(old_elapsed > trie_elapsed);
}

TEST_CASE("[TRIES] Range Count Queries: old vs Trie method", "[tries][performance][range]") {
	constexpr size_t n = 30000;
	constexpr size_t q = 2000;

	map<size_t, int> umap;
	XFastTrie<int> xft(n);

	for (size_t i = 0; i < n; i++) {
		size_t key = i * 2;
		int value = static_cast<int>(i);
		umap[key] = value;
		xft.insert(key, value);
	}

	const auto old_start = chrono::high_resolution_clock::now();
	size_t old_total = 0;
	for (size_t i = 0; i < q; i++) {
		size_t a = i * 10 + 3;
		size_t b = a + 40;
		size_t cnt = 0;
		for (const auto &kv : umap) {
			size_t k = kv.first;
			if (k >= a && k <= b) cnt++;
		}
		old_total += cnt;
	}
	const double old_elapsed = chrono::duration<double>(chrono::high_resolution_clock::now() - old_start).count();

	const auto trie_start = chrono::high_resolution_clock::now();
	size_t trie_total = 0;
	for (size_t i = 0; i < q; i++) {
		size_t a = i * 10 + 3;
		size_t b = a + 40;
		size_t cnt = 0;
		auto it = xft.successor(a);
		while (it != xft.end() && it.key() <= b) {
			cnt++;
			++it;
		}
		trie_total += cnt;
	}
	const double trie_elapsed = chrono::duration<double>(chrono::high_resolution_clock::now() - trie_start).count();

	cout << fixed << setprecision(3);
	cout << "[OLD PERFORMANCE: RANGE COUNT] " << old_elapsed << " seconds\n";
	cout << "[TRIE PERFORMANCE: RANGE COUNT] " << trie_elapsed << " seconds\n";
	REQUIRE(old_elapsed > trie_elapsed);
}

TEST_CASE("[TRIES] Mixed Neighbor Queries: old vs Trie method", "[tries][performance][neighbor]") {
	constexpr size_t n = 30000;
	constexpr size_t q = 3000;

	map<size_t, int> map;
	XFastTrie<int> xft(n);

	for (size_t i = 0; i < n; i++) {
		size_t key = i * 2;
		int value = static_cast<int>(i);
		map[key] = value;
		xft.insert(key, value);
	}

	const auto old_start = chrono::high_resolution_clock::now();
	size_t old_sum = 0;
	for (size_t i = 0; i < q; i++) {
		size_t query = i * 3 + 1;
		size_t best_le = 0, best_ge = std::numeric_limits<size_t>::max();
		bool has_le = false, has_ge = false;
		for (const auto &kv : map) {
			size_t k = kv.first;
			if (k <= query) { if (!has_le || k > best_le) best_le = k; has_le = true; }
			if (k >= query) { if (!has_ge || k < best_ge) best_ge = k; has_ge = true; }
		}
		if (has_le) old_sum += map[best_le];
		if (has_ge) old_sum += map[best_ge];
	}
	const double old_elapsed = chrono::duration<double>(chrono::high_resolution_clock::now() - old_start).count();

	const auto trie_start = chrono::high_resolution_clock::now();
	size_t trie_sum = 0;
	for (size_t i = 0; i < q; i++) {
		size_t query = i * 3 + 1;
		auto pre = xft.predecessor(query);
		auto suc = xft.successor(query);
		if (pre != xft.end()) trie_sum += pre.value();
		if (suc != xft.end()) trie_sum += suc.value();
	}
	const double trie_elapsed = chrono::duration<double>(chrono::high_resolution_clock::now() - trie_start).count();

	cout << fixed << setprecision(3);
	cout << "[OLD PERFORMANCE: MIXED NEIGHBOR] " << old_elapsed << " seconds\n";
	cout << "[TRIE PERFORMANCE: MIXED NEIGHBOR] " << trie_elapsed << " seconds\n";
	REQUIRE(old_elapsed > trie_elapsed);
}

*/

// INSERTION SECTION
TEST_CASE("[TRIES] Trie insertion test cases", "[X-fast_trie][insertion]") {
	// New Trie instance sized for keys up to 2
	XFastTrie<size_t, int> xft(2);

	// Collect and insert varia	bles into Trie
	int insert_this_value = 67;
	xft.insert(1, insert_this_value); // Make 1 = 67

	constexpr size_t key_to_check = 1;
	REQUIRE(xft.contains(key_to_check)); // Confirm insertion 1 = 67
}

TEST_CASE("[TRIES] Trie multiple insertions and ordering", "[X-fast_trie][insertion-order]") {
	// New Trie instance sized for keys up to 3
	XFastTrie<size_t, int> xft(3);

	// Collect and insert variables into Trie
	int insert_this_value = 1, insert_this_value_2 = 3, insert_this_value_3 = 2;
	xft.insert(1, insert_this_value);
	xft.insert(3, insert_this_value_2);
	xft.insert(2, insert_this_value_3);

	// Confirm keys 1, 3, 2 are occupied
	REQUIRE(xft.contains(1));
	REQUIRE(xft.contains(3));
	REQUIRE(xft.contains(2));

	// Create an iterator
	auto it = xft.begin();

	// Confirm the keys and values are ordered
	REQUIRE(it->first == 1);
	REQUIRE(it->second == 1);

	it = next(it);
	REQUIRE(it->first == 2);
	REQUIRE(it->second == 2);

	it = next(it);
	REQUIRE(it->first == 3);
	REQUIRE(it->second == 3);
}

// ITERATION SECTION
TEST_CASE("[TRIES] Trie iteration test cases", "[X-fast_trie][iteration]") {
	// New Trie instance sized for keys up to 3
	XFastTrie<size_t, int> xft(3);

	// Collect and insert variables into Trie
	int insert_this_value = 1, insert_this_value_2 = 2, insert_this_value_3 = 3;
	xft.insert(1, insert_this_value);
	xft.insert(2, insert_this_value_2);
	xft.insert(3, insert_this_value_3);

	// Count the number of keys via iterator
	size_t count = 0;
	for (auto it = xft.begin(); it != xft.end(); ++it) count++;
	REQUIRE(count == 3);
}

// REMOVAL SECTION (ERROR)
TEST_CASE("[TRIES] Trie removal test cases", "[X-fast_trie][removal]") {
	// New Trie instance sized for keys up to 1
	XFastTrie<size_t, int> xft(1);

	// Collect and insert variables into Trie
	int insert_this_value = 67;
	xft.insert(1, insert_this_value); // Make 1 = 67

	constexpr size_t key_to_check = 1;
	REQUIRE(xft.contains(key_to_check)); // Confirm insertion 1 = 67

	xft.erase(key_to_check); // Remove key 1

	REQUIRE(xft.contains(key_to_check) == false); // Confirm key 1 no longer exists
}

// FIND SECTION
TEST_CASE("[TRIES] Trie find test cases", "[X-fast_trie][find]") {
	// New Trie instance sized for keys up to 2
	XFastTrie<size_t, int> xft(2);

	// Collect and insert variables into Trie
	int insert_this_value = 1, insert_this_value_2 = 2;
	xft.insert(4, insert_this_value);
	xft.insert(8, insert_this_value_2);

	// Key 4 = 1
	auto it = xft.find(4);
	REQUIRE(it != xft.end());
	REQUIRE(it->second == 1);

	// Key 99 does not exist
	auto it2 = xft.find(99);
	REQUIRE(it2 == xft.end());
}

// PREDECESSOR AND SUCCESSOR SECTION
TEST_CASE("[TRIES] Trie predecessor test cases", "[X-fast_trie][predecessor]") {
	// New Trie instance sized for keys up to 3
	XFastTrie<size_t, int> xft(3);

	// Collect and insert variables into Trie
	int insert_this_value = 1, insert_this_value_2 = 2, insert_this_value_3 = 3;
	xft.insert(10, insert_this_value);
	xft.insert(20, insert_this_value_2);
	xft.insert(30, insert_this_value_3);

	// Get the key before key 20
	auto pre = xft.predecessor(30);
	REQUIRE(pre->first == 20);
	REQUIRE(pre->second == 2);

	// Get the key before key 5 (does not exist)
	auto none = xft.predecessor(10);
	REQUIRE(none == xft.end());
}

TEST_CASE("[TRIES] Trie successor test cases", "[X-fast_trie][successor]") {
	// New Trie instance sized for keys up to 3
	XFastTrie<size_t, int> xft(3);

	// Collect and insert variables into Trie
	int insert_this_value = 1, insert_this_value_2 = 2, insert_this_value_3 = 3;
	xft.insert(10, insert_this_value);
	xft.insert(20, insert_this_value_2);
	xft.insert(30, insert_this_value_3);

	auto suc = xft.successor(10);
	REQUIRE(suc->first == 20);
	REQUIRE(suc->second == 2);

	auto none = xft.successor(30);
	REQUIRE(none == xft.end());
}

// EDGE-CASES SECTION (ERROR)
TEST_CASE("[TRIES] Trie contains and empty edge cases", "[X-fast_trie][contains-empty]") {
	// New Trie instance sized for keys up to 1
	XFastTrie<size_t, int> xft(1);

	// Check key 100 is not in empty Trie
	REQUIRE(xft.contains(100) == false);

	// Collect and insert variables into Trie
	int insert_this_value = 5;
	xft.insert(3, insert_this_value);
	REQUIRE(xft.contains(3)); // Confirm key 3 exists

	// Remove key 3
	xft.erase(3);
	REQUIRE(xft.contains(3) == false); // Confirm key 3 no longer exists
}

TEST_CASE("[TRIES] Trie edge-case removal of last element", "[X-fast_trie][last-removal]") {
	// New Trie instance sized for keys up to 1
	XFastTrie<size_t, int> xft(1);

	// Collect and insert variables into Trie
	int insert_this_value = 9;
	xft.insert(1, insert_this_value);

	// Confirm insertion 1 = 67
	REQUIRE(xft.contains(1));

	// Remove key 1
	xft.erase(1);

	// Confirm that key 1 no longer exists
	REQUIRE(xft.contains(1) == false);

	// Verify that the Trie is now empty where begin == end
	REQUIRE(xft.begin() == xft.end());
}

// ---New test cases for sorting verification---
TEST_CASE("X-fast_trie N element size_t-key-sort test", "[sorting]") {
	size_t N = 1000;
	size_t N2delete = N/2;
	RandomDatasetGenerator rdg(N);
	XFastTrie<size_t,int> xft(N);
	std::map<size_t,int> dup_free_and_sorted;
	for(size_t i = 0; i < N; i++) {
		dup_free_and_sorted.emplace(rdg.random_size_ts[i],rdg.random_ints[i]);
		xft.insert(rdg.random_size_ts[i],rdg.random_ints[i]);
	}

	std::vector<size_t> keys;
	for(auto iter = dup_free_and_sorted.begin(); iter != dup_free_and_sorted.end(); ++iter) {
		keys.push_back(iter->first);
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(keys.begin(), keys.end(), g);


	for(size_t i = 0; i < N2delete; i++) {
		size_t key2delete = keys[i];
		dup_free_and_sorted.erase(key2delete);
		bool erased_success = xft.erase(key2delete);
		if (!erased_success) {
			FAIL("Xft failed to erase key: " << key2delete << ", which exists in the dataset.");
		}
	}
	bool is_sorted = true;
	auto iter_stl_map = dup_free_and_sorted.begin();
	auto iter = xft.begin();
	size_t i = 0;
	while(iter_stl_map != dup_free_and_sorted.end() and iter != xft.end()){
		if(iter_stl_map->first != iter.key()) {
			is_sorted = false;
			FAIL("MISMATCH at index " << i << ". Map expects: " << iter_stl_map->first << " but xft has: " << iter.key());
			break;
		}
		++iter_stl_map;
		++iter;
		++i;
	}

	if (is_sorted) {
		// If we are here, the data matched so far, but xft ended too soon.
		if (iter_stl_map != dup_free_and_sorted.end()) {
			FAIL("xft is broken. Map has " << dup_free_and_sorted.size() << " items, but xft iteration stopped after " << i);
		}
	}

	REQUIRE(iter_stl_map == dup_free_and_sorted.end());
	REQUIRE(iter == xft.end());
	REQUIRE(is_sorted);
}

TEST_CASE("X-fast_trie N element int-key-sort test", "[sorting]") {
	size_t N = 1000;
	size_t N2delete = N/2;
	RandomDatasetGenerator rdg(N);
	XFastTrie<int,size_t> xft(N);
	std::map<int,size_t> dup_free_and_sorted;
	for(size_t i = 0; i < N; i++) {
		dup_free_and_sorted.emplace(rdg.random_ints[i],rdg.random_size_ts[i]);
		xft.insert(rdg.random_ints[i],rdg.random_size_ts[i]);
	}

	std::vector<int> keys;
	for(auto iter = dup_free_and_sorted.begin(); iter != dup_free_and_sorted.end(); ++iter) {
		keys.push_back(iter->first);
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(keys.begin(), keys.end(), g);


	for(size_t i = 0; i < N2delete; i++) {
		int key2delete = keys[i];
		dup_free_and_sorted.erase(key2delete);
		bool erased_success = xft.erase(key2delete);
		if (!erased_success) {
			FAIL("Xft failed to erase key: " << key2delete << ", which exists in the dataset.");
		}
	}
	bool is_sorted = true;
	auto iter_stl_map = dup_free_and_sorted.begin();
	auto iter = xft.begin();
	size_t i = 0;
	while(iter_stl_map != dup_free_and_sorted.end() and iter != xft.end()){
		if(iter_stl_map->first != iter.key()) {
			is_sorted = false;
			FAIL("MISMATCH at index " << i << ". Map expects: " << iter_stl_map->first << " but xft has: " << iter.key());
			break;
		}
		++iter_stl_map;
		++iter;
		++i;
	}

	if (is_sorted) {
		// If we are here, the data matched so far, but xft ended too soon.
		if (iter_stl_map != dup_free_and_sorted.end()) {
			FAIL("xft is broken. Map has " << dup_free_and_sorted.size() << " items, but xft iteration stopped after " << i);
		}
	}

	REQUIRE(iter_stl_map == dup_free_and_sorted.end());
	REQUIRE(iter == xft.end());
	REQUIRE(is_sorted);
}

TEST_CASE("Batch_N_Hash_List N element size_t-key-sort test", "[sorting]") {
	size_t N = 1000;
	size_t N2delete = N/2;
	RandomDatasetGenerator rdg(N);
	Batch_N_Hash_List<size_t,int> bnhl(N);
	std::map<size_t,int> dup_free_and_sorted;
	for(size_t i = 0; i < N; i++) {
		dup_free_and_sorted.emplace(rdg.random_size_ts[i],rdg.random_ints[i]);
		bnhl.addHead(rdg.random_size_ts[i],rdg.random_ints[i]);
	}

	std::vector<size_t> keys;
	for(auto iter = dup_free_and_sorted.begin(); iter != dup_free_and_sorted.end(); ++iter) {
		keys.push_back(iter->first);
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(keys.begin(), keys.end(), g);


	for(size_t i = 0; i < N2delete; i++) {
		size_t key2delete = keys[i];
		dup_free_and_sorted.erase(key2delete);
		bool erased_success = bnhl.remove(key2delete);
		if (!erased_success) {
			FAIL("Bnhl failed to erase key: " << key2delete << ", which exists in the dataset.");
		}
	}
	bnhl.sort_keys();
	bool is_sorted = true;
	auto iter_stl_map = dup_free_and_sorted.begin();
	auto iter = bnhl.begin();
	size_t i = 0;
	while(iter_stl_map != dup_free_and_sorted.end() and iter != bnhl.end()){
		if(iter_stl_map->first != iter.key()) {
			is_sorted = false;
			FAIL("MISMATCH at index " << i << ". Map expects: " << iter_stl_map->first << " but bnhl has: " << iter.key());
			break;
		}
		++iter_stl_map;
		++iter;
		++i;
	}

	if (is_sorted) {
		// If we are here, the data matched so far, but bnhl ended too soon.
		if (iter_stl_map != dup_free_and_sorted.end()) {
			FAIL("bnhl is broken. Map has " << dup_free_and_sorted.size() << " items, but bnhl iteration stopped after " << i);
		}
	}

	REQUIRE(iter_stl_map == dup_free_and_sorted.end());
	REQUIRE(iter == bnhl.end());
	REQUIRE(is_sorted);
}

TEST_CASE("Batch N Hash List N element int-key-sort test", "[sorting]") {
	size_t N = 1000;
	size_t N2delete = N/2;
	RandomDatasetGenerator rdg(N);
	Batch_N_Hash_List<int, size_t> bnhl(N);
	std::map<int, size_t> dup_free_and_sorted;
	for(size_t i = 0; i < N; i++) {
		dup_free_and_sorted.emplace(rdg.random_ints[i],rdg.random_size_ts[i]);
		bnhl.addHead(rdg.random_ints[i],rdg.random_size_ts[i]);
	}

	std::vector<int> keys;
	for(auto iter = dup_free_and_sorted.begin(); iter != dup_free_and_sorted.end(); ++iter) {
		keys.push_back(iter->first);
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(keys.begin(), keys.end(), g);


	for(size_t i = 0; i < N2delete; i++) {
		int key2delete = keys[i];
		dup_free_and_sorted.erase(key2delete);
		bool erased_success = bnhl.remove(key2delete);
		if (!erased_success) {
			FAIL("Bnhl failed to erase key: " << key2delete << ", which exists in the dataset.");
		}
	}
	bnhl.sort_keys();
	bool is_sorted = true;
	auto iter_stl_map = dup_free_and_sorted.begin();
	auto iter = bnhl.begin();
	size_t i = 0;
	while(iter_stl_map != dup_free_and_sorted.end() and iter != bnhl.end()){
		if(iter_stl_map->first != iter.key()) {
			is_sorted = false;
			FAIL("MISMATCH at index " << i << ". Map expects: " << iter_stl_map->first << " but bnhl has: " << iter.key());
			break;
		}
		++iter_stl_map;
		++iter;
		++i;
	}

	if (is_sorted) {
		// If we are here, the data matched so far, but bnhl ended too soon.
		if (iter_stl_map != dup_free_and_sorted.end()) {
			FAIL("bnhl is broken. Map has " << dup_free_and_sorted.size() << " items, but bnhl iteration stopped after " << i);
		}
	}

	REQUIRE(iter_stl_map == dup_free_and_sorted.end());
	REQUIRE(iter == bnhl.end());
	REQUIRE(is_sorted);
}

TEST_CASE("Radix flat map N element size_t-key-sort test", "[sorting]") {
	size_t N = 1000;
	size_t N2delete = N/2;
	RandomDatasetGenerator rdg(N);
	Radix_Flat_Map<size_t,int> rfm;
	std::map<size_t,int> dup_free_and_sorted;
	for(size_t i = 0; i < N; i++) {
		dup_free_and_sorted.emplace(rdg.random_size_ts[i],rdg.random_ints[i]);
		rfm.insert(rdg.random_size_ts[i],rdg.random_ints[i]);
	}

	std::vector<size_t> keys;
	for(auto iter = dup_free_and_sorted.begin(); iter != dup_free_and_sorted.end(); ++iter) {
		keys.push_back(iter->first);
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(keys.begin(), keys.end(), g);


	for(size_t i = 0; i < N2delete; i++) {
		size_t key2delete = keys[i];
		dup_free_and_sorted.erase(key2delete);
		bool erased_success = rfm.erase(key2delete);
		if (!erased_success) {
			FAIL("Rfm failed to erase key: " << key2delete << ", which exists in the dataset.");
		}
	}
	bool is_sorted = true;
	auto iter_stl_map = dup_free_and_sorted.begin();
	auto iter = rfm.begin();
	size_t i = 0;
	while(iter_stl_map != dup_free_and_sorted.end() and iter != rfm.end()){
		if(iter_stl_map->first != iter->first) {
			is_sorted = false;
			FAIL("MISMATCH at index " << i << ". Map expects: " << iter_stl_map->first << " but rfm has: " << iter->first);
			break;
		}
		++iter_stl_map;
		++iter;
		++i;
	}

	if (is_sorted) {
		// If we are here, the data matched so far, but rfm ended too soon.
		if (iter_stl_map != dup_free_and_sorted.end()) {
			FAIL("rfm is broken. Map has " << dup_free_and_sorted.size() << " items, but rfm iteration stopped after " << i);
		}
	}

	REQUIRE(iter_stl_map == dup_free_and_sorted.end());
	REQUIRE(iter == rfm.end());
	REQUIRE(is_sorted);
}

TEST_CASE("Radix_flat_map N element int-key-sort test", "[sorting]") {
	size_t N = 1000;
	size_t N2delete = N/2;
	RandomDatasetGenerator rdg(N);
	Radix_Flat_Map<int, size_t> rfm;
	std::map<int, size_t> dup_free_and_sorted;
	for(size_t i = 0; i < N; i++) {
		dup_free_and_sorted.emplace(rdg.random_ints[i],rdg.random_size_ts[i]);
		rfm.insert(rdg.random_ints[i],rdg.random_size_ts[i]);
	}

	std::vector<int> keys;
	for(auto iter = dup_free_and_sorted.begin(); iter != dup_free_and_sorted.end(); ++iter) {
		keys.push_back(iter->first);
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(keys.begin(), keys.end(), g);


	for(size_t i = 0; i < N2delete; i++) {
		int key2delete = keys[i];
		dup_free_and_sorted.erase(key2delete);
		bool erased_success = rfm.erase(key2delete);
		if (!erased_success) {
			FAIL("Rfm failed to erase key: " << key2delete << ", which exists in the dataset.");
		}
	}
	bool is_sorted = true;
	auto iter_stl_map = dup_free_and_sorted.begin();
	auto iter = rfm.begin();
	size_t i = 0;
	while(iter_stl_map != dup_free_and_sorted.end() and iter != rfm.end()){
		if(iter_stl_map->first != iter->first) {
			is_sorted = false;
			FAIL("MISMATCH at index " << i << ". Map expects: " << iter_stl_map->first << " but rfm has: " << iter->first);
			break;
		}
		++iter_stl_map;
		++iter;
		++i;
	}

	if (is_sorted) {
		// If we are here, the data matched so far, but rfm ended too soon.
		if (iter_stl_map != dup_free_and_sorted.end()) {
			FAIL("Rfm is broken. Map has " << dup_free_and_sorted.size() << " items, but rfm iteration stopped after " << i);
		}
	}

	REQUIRE(iter_stl_map == dup_free_and_sorted.end());
	REQUIRE(iter == rfm.end());
	REQUIRE(is_sorted);
}

TEST_CASE("Treap N element size_t-key-sort test", "[sorting]") {
	size_t N = 1000;
	size_t N2delete = N/2;
	RandomDatasetGenerator rdg(N);
	Treap<size_t,int> treap;
	std::map<size_t,int> dup_free_and_sorted;
	for(size_t i = 0; i < N; i++) {
		dup_free_and_sorted.emplace(rdg.random_size_ts[i],rdg.random_ints[i]);
		treap.insert(rdg.random_size_ts[i],rdg.random_ints[i]);
	}

	std::vector<size_t> keys;
	for(auto iter = dup_free_and_sorted.begin(); iter != dup_free_and_sorted.end(); ++iter) {
		keys.push_back(iter->first);
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(keys.begin(), keys.end(), g);


	for(size_t i = 0; i < N2delete; i++) {
		size_t key2delete = keys[i];
		dup_free_and_sorted.erase(key2delete);
		bool erased_success = treap.erase(key2delete);
		if (!erased_success) {
			FAIL("Treap failed to erase key: " << key2delete << ", which exists in the dataset.");
		}
	}
	bool is_sorted = true;
	auto iter_stl_map = dup_free_and_sorted.begin();
	auto iter = treap.begin();
	size_t i = 0;
	while(iter_stl_map != dup_free_and_sorted.end() and iter != treap.end()){
		if(iter_stl_map->first != iter->first) {
			is_sorted = false;
			FAIL("MISMATCH at index " << i << ". Map expects: " << iter_stl_map->first << " but treap has: " << iter->first);
			break;
		}
		++iter_stl_map;
		++iter;
		++i;
	}

	if (is_sorted) {
		// If we are here, the data matched so far, but treap ended too soon.
		if (iter_stl_map != dup_free_and_sorted.end()) {
			FAIL("Treap is broken. Map has " << dup_free_and_sorted.size() << " items, but treap iteration stopped after " << i);
		}
	}

	REQUIRE(iter_stl_map == dup_free_and_sorted.end());
	REQUIRE(iter == treap.end());
	REQUIRE(is_sorted);
}

TEST_CASE("Treap N element int-key-sort test", "[sorting]") {
	size_t N = 1000;
	size_t N2delete = N/2;
	RandomDatasetGenerator rdg(N);
	Radix_Flat_Map<int, size_t> treap;
	std::map<int, size_t> dup_free_and_sorted;
	for(size_t i = 0; i < N; i++) {
		dup_free_and_sorted.emplace(rdg.random_ints[i],rdg.random_size_ts[i]);
		treap.insert(rdg.random_ints[i],rdg.random_size_ts[i]);
	}

	std::vector<int> keys;
	for(auto iter = dup_free_and_sorted.begin(); iter != dup_free_and_sorted.end(); ++iter) {
		keys.push_back(iter->first);
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(keys.begin(), keys.end(), g);


	for(size_t i = 0; i < N2delete; i++) {
		int key2delete = keys[i];
		dup_free_and_sorted.erase(key2delete);
		bool erased_success = treap.erase(key2delete);
		if (!erased_success) {
			FAIL("Treap failed to erase key: " << key2delete << ", which exists in the dataset.");
		}
	}
	bool is_sorted = true;
	auto iter_stl_map = dup_free_and_sorted.begin();
	auto iter = treap.begin();
	size_t i = 0;
	while(iter_stl_map != dup_free_and_sorted.end() and iter != treap.end()){
		if(iter_stl_map->first != iter->first) {
			is_sorted = false;
			FAIL("MISMATCH at index " << i << ". Map expects: " << iter_stl_map->first << " but treap has: " << iter->first);
			break;
		}
		++iter_stl_map;
		++iter;
		++i;
	}

	if (is_sorted) {
		// If we are here, the data matched so far, but treap ended too soon.
		if (iter_stl_map != dup_free_and_sorted.end()) {
			FAIL("Treap is broken. Map has " << dup_free_and_sorted.size() << " items, but treap iteration stopped after " << i);
		}
	}

	REQUIRE(iter_stl_map == dup_free_and_sorted.end());
	REQUIRE(iter == treap.end());
	REQUIRE(is_sorted);
}

TEST_CASE("AVL Tree N element size_t-key-sort test", "[sorting]") {
	size_t N = 1000;
	size_t N2delete = N/2;
	RandomDatasetGenerator rdg(N);
	AVL_Tree<size_t,int> avl_tree;
	std::map<size_t,int> dup_free_and_sorted;
	for(size_t i = 0; i < N; i++) {
		dup_free_and_sorted.emplace(rdg.random_size_ts[i],rdg.random_ints[i]);
		avl_tree.insert(rdg.random_size_ts[i],rdg.random_ints[i]);
	}

	std::vector<size_t> keys;
	for(auto iter = dup_free_and_sorted.begin(); iter != dup_free_and_sorted.end(); ++iter) {
		keys.push_back(iter->first);
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(keys.begin(), keys.end(), g);


	for(size_t i = 0; i < N2delete; i++) {
		size_t key2delete = keys[i];
		dup_free_and_sorted.erase(key2delete);
		bool erased_success = avl_tree.erase(key2delete);
		if (!erased_success) {
			FAIL("avl_tree failed to erase key: " << key2delete << ", which exists in the dataset.");
		}
	}
	bool is_sorted = true;
	auto iter_stl_map = dup_free_and_sorted.begin();
	auto iter = avl_tree.begin();
	size_t i = 0;
	while(iter_stl_map != dup_free_and_sorted.end() and iter != avl_tree.end()){
		if(iter_stl_map->first != iter->first) {
			is_sorted = false;
			FAIL("MISMATCH at index " << i << ". Map expects: " << iter_stl_map->first << " but avl_tree has: " << iter->first);
			break;
		}
		++iter_stl_map;
		++iter;
		++i;
	}

	if (is_sorted) {
		// If we are here, the data matched so far, but avl_tree ended too soon.
		if (iter_stl_map != dup_free_and_sorted.end()) {
			FAIL("avl_tree is broken. Map has " << dup_free_and_sorted.size() << " items, but avl_tree iteration stopped after " << i);
		}
	}

	REQUIRE(iter_stl_map == dup_free_and_sorted.end());
	REQUIRE(iter == avl_tree.end());
	REQUIRE(is_sorted);
}

TEST_CASE("AVL Tree N element int-key-sort test", "[sorting]") {
	size_t N = 1000;
	size_t N2delete = N/2;
	RandomDatasetGenerator rdg(N);
	AVL_Tree<int, size_t> avl_tree;
	std::map<int, size_t> dup_free_and_sorted;
	for(size_t i = 0; i < N; i++) {
		dup_free_and_sorted.emplace(rdg.random_ints[i],rdg.random_size_ts[i]);
		avl_tree.insert(rdg.random_ints[i],rdg.random_size_ts[i]);
	}

	std::vector<int> keys;
	for(auto iter = dup_free_and_sorted.begin(); iter != dup_free_and_sorted.end(); ++iter) {
		keys.push_back(iter->first);
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(keys.begin(), keys.end(), g);


	for(size_t i = 0; i < N2delete; i++) {
		int key2delete = keys[i];
		dup_free_and_sorted.erase(key2delete);
		bool erased_success = avl_tree.erase(key2delete);
		if (!erased_success) {
			FAIL("avl_tree failed to erase key: " << key2delete << ", which exists in the dataset.");
		}
	}
	bool is_sorted = true;
	auto iter_stl_map = dup_free_and_sorted.begin();
	auto iter = avl_tree.begin();
	size_t i = 0;
	while(iter_stl_map != dup_free_and_sorted.end() and iter != avl_tree.end()){
		if(iter_stl_map->first != iter->first) {
			is_sorted = false;
			FAIL("MISMATCH at index " << i << ". Map expects: " << iter_stl_map->first << " but avl_tree has: " << iter->first);
			break;
		}
		++iter_stl_map;
		++iter;
		++i;
	}

	if (is_sorted) {
		// If we are here, the data matched so far, but avl_tree ended too soon.
		if (iter_stl_map != dup_free_and_sorted.end()) {
			FAIL("avl_tree is broken. Map has " << dup_free_and_sorted.size() << " items, but avl_tree iteration stopped after " << i);
		}
	}

	REQUIRE(iter_stl_map == dup_free_and_sorted.end());
	REQUIRE(iter == avl_tree.end());
	REQUIRE(is_sorted);
}

TEST_CASE("Hash Map AVL Tree N element size_t-key-sort test", "[sorting]") {
	size_t N = 1000;
	size_t N2delete = N/2;
	RandomDatasetGenerator rdg(N);
	Hash_Map_AVL_Tree<size_t,int> hm_avl_tree(N);
	std::map<size_t,int> dup_free_and_sorted;
	for(size_t i = 0; i < N; i++) {
		dup_free_and_sorted.emplace(rdg.random_size_ts[i],rdg.random_ints[i]);
		hm_avl_tree.insert(rdg.random_size_ts[i],rdg.random_ints[i]);
	}

	std::vector<size_t> keys;
	for(auto iter = dup_free_and_sorted.begin(); iter != dup_free_and_sorted.end(); ++iter) {
		keys.push_back(iter->first);
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(keys.begin(), keys.end(), g);


	for(size_t i = 0; i < N2delete; i++) {
		size_t key2delete = keys[i];
		dup_free_and_sorted.erase(key2delete);
		bool erased_success = hm_avl_tree.erase(key2delete);
		if (!erased_success) {
			FAIL("hm_avl_tree failed to erase key: " << key2delete << ", which exists in the dataset.");
		}
	}
	bool is_sorted = true;
	auto iter_stl_map = dup_free_and_sorted.begin();
	auto iter = hm_avl_tree.begin();
	size_t i = 0;
	while(iter_stl_map != dup_free_and_sorted.end() and iter != hm_avl_tree.end()){
		if(iter_stl_map->first != iter->first) {
			is_sorted = false;
			FAIL("MISMATCH at index " << i << ". Map expects: " << iter_stl_map->first << " but hm_avl_tree has: " << iter->first);
			break;
		}
		++iter_stl_map;
		++iter;
		++i;
	}

	if (is_sorted) {
		// If we are here, the data matched so far, but hm_avl_tree ended too soon.
		if (iter_stl_map != dup_free_and_sorted.end()) {
			FAIL("hm_avl_tree is broken. Map has " << dup_free_and_sorted.size() << " items, but hm_avl_tree iteration stopped after " << i);
		}
	}

	REQUIRE(iter_stl_map == dup_free_and_sorted.end());
	REQUIRE(iter == hm_avl_tree.end());
	REQUIRE(is_sorted);
}

TEST_CASE("Hash Map AVL Tree N element int-key-sort test", "[sorting]") {
	size_t N = 1000;
	size_t N2delete = N/2;
	RandomDatasetGenerator rdg(N);
	Hash_Map_AVL_Tree<int, size_t> hm_avl_tree(N);
	std::map<int, size_t> dup_free_and_sorted;
	for(size_t i = 0; i < N; i++) {
		dup_free_and_sorted.emplace(rdg.random_ints[i],rdg.random_size_ts[i]);
		hm_avl_tree.insert(rdg.random_ints[i],rdg.random_size_ts[i]);
	}

	std::vector<int> keys;
	for(auto iter = dup_free_and_sorted.begin(); iter != dup_free_and_sorted.end(); ++iter) {
		keys.push_back(iter->first);
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(keys.begin(), keys.end(), g);


	for(size_t i = 0; i < N2delete; i++) {
		int key2delete = keys[i];
		dup_free_and_sorted.erase(key2delete);
		bool erased_success = hm_avl_tree.erase(key2delete);
		if (!erased_success) {
			FAIL("hm_avl_tree failed to erase key: " << key2delete << ", which exists in the dataset.");
		}
	}
	bool is_sorted = true;
	auto iter_stl_map = dup_free_and_sorted.begin();
	auto iter = hm_avl_tree.begin();
	size_t i = 0;
	while(iter_stl_map != dup_free_and_sorted.end() and iter != hm_avl_tree.end()){
		if(iter_stl_map->first != iter->first) {
			is_sorted = false;
			FAIL("MISMATCH at index " << i << ". Map expects: " << iter_stl_map->first << " but hm_avl_tree has: " << iter->first);
			break;
		}
		++iter_stl_map;
		++iter;
		++i;
	}

	if (is_sorted) {
		// If we are here, the data matched so far, but hm_avl_tree ended too soon.
		if (iter_stl_map != dup_free_and_sorted.end()) {
			FAIL("hm_avl_tree is broken. Map has " << dup_free_and_sorted.size() << " items, but hm_avl_tree iteration stopped after " << i);
		}
	}

	REQUIRE(iter_stl_map == dup_free_and_sorted.end());
	REQUIRE(iter == hm_avl_tree.end());
	REQUIRE(is_sorted);
}
