#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <iostream>
#include <iomanip>

//fastest single-threaded map candidates for all int and float types
#include "Radix_Flat_Map.h" //fast with read-heavy workloads
#include "Batch_N_Hash_List.h" //fast with write-heavy workloads or batch lookup only
#include <map> //best for abstract data types
#include "X-fast_Trie.h" //best for mixed workloads
#include "AVL_Tree.h" //could be better for abstract data types

#include "SmallTestDataset.h" //contains no duplicates for easier debugging; has 100 truly-random size_t and strings
using namespace std; //todo: remove when finished

// PERFORMANCE SECTION
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

// INSERTION SECTION
TEST_CASE("[TRIES] Trie insertion test cases", "[tries][insertion]") {
	// New Trie instance sized for keys up to 2
	XFastTrie<int> xft(2);

	// Collect and insert varia	bles into Trie
	int insert_this_value = 67;
	xft.insert(1, insert_this_value); // Make 1 = 67

	constexpr size_t key_to_check = 1;
	REQUIRE(xft.contains(key_to_check)); // Confirm insertion 1 = 67
}

TEST_CASE("[TRIES] Trie multiple insertions and ordering", "[tries][insertion-order]") {
	// New Trie instance sized for keys up to 3
	XFastTrie<int> xft(3);

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
	REQUIRE(it.key() == 1);
	REQUIRE(it.value() == 1);

	it = next(it);
	REQUIRE(it.key() == 2);
	REQUIRE(it.value() == 2);

	it = next(it);
	REQUIRE(it.key() == 3);
	REQUIRE(it.value() == 3);
}

// ITERATION SECTION
TEST_CASE("[TRIES] Trie iteration test cases", "[tries][iteration]") {
	// New Trie instance sized for keys up to 3
	XFastTrie<int> xft(3);

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
TEST_CASE("[TRIES] Trie removal test cases", "[tries][removal]") {
	// New Trie instance sized for keys up to 1
	XFastTrie<int> xft(1);

	// Collect and insert variables into Trie
	int insert_this_value = 67;
	xft.insert(1, insert_this_value); // Make 1 = 67

	constexpr size_t key_to_check = 1;
	REQUIRE(xft.contains(key_to_check)); // Confirm insertion 1 = 67

	xft.remove(key_to_check); // Remove key 1

	REQUIRE(xft.contains(key_to_check) == false); // Confirm key 1 no longer exists
}

// FIND SECTION
TEST_CASE("[TRIES] Trie find test cases", "[tries][find]") {
	// New Trie instance sized for keys up to 2
	XFastTrie<int> xft(2);

	// Collect and insert variables into Trie
	int insert_this_value = 1, insert_this_value_2 = 2;
	xft.insert(4, insert_this_value);
	xft.insert(8, insert_this_value_2);

	// Key 4 = 1
	auto it = xft.find(4);
	REQUIRE(it != xft.end());
	REQUIRE(it.value() == 1);

	// Key 99 does not exist
	auto it2 = xft.find(99);
	REQUIRE(it2 == xft.end());
}

// PREDECESSOR AND SUCCESSOR SECTION
TEST_CASE("[TRIES] Trie predecessor test cases", "[tries][predecessor]") {
	// New Trie instance sized for keys up to 3
	XFastTrie<int> xft(3);

	// Collect and insert variables into Trie
	int insert_this_value = 1, insert_this_value_2 = 2, insert_this_value_3 = 3;
	xft.insert(10, insert_this_value);
	xft.insert(20, insert_this_value_2);
	xft.insert(30, insert_this_value_3);

	// Get the key before key 20
	auto pre = xft.predecessor(30);
	REQUIRE(pre.key() == 20);
	REQUIRE(pre.value() == 2);

	// Get the key before key 5 (does not exist)
	auto none = xft.predecessor(10);
	REQUIRE(none == xft.end());
}

TEST_CASE("[TRIES] Trie successor test cases", "[tries][successor]") {
	// New Trie instance sized for keys up to 3
	XFastTrie<int> xft(3);

	// Collect and insert variables into Trie
	int insert_this_value = 1, insert_this_value_2 = 2, insert_this_value_3 = 3;
	xft.insert(10, insert_this_value);
	xft.insert(20, insert_this_value_2);
	xft.insert(30, insert_this_value_3);

	auto suc = xft.successor(10);
	REQUIRE(suc.key() == 20);
	REQUIRE(suc.value() == 2);

	auto none = xft.successor(30);
	REQUIRE(none == xft.end());
}

// EDGE-CASES SECTION (ERROR)
TEST_CASE("[TRIES] Trie contains and empty edge cases", "[tries][contains-empty]") {
	// New Trie instance sized for keys up to 1
	XFastTrie<int> xft(1);

	// Check key 100 is not in empty Trie
	REQUIRE(xft.contains(100) == false);

	// Collect and insert variables into Trie
	int insert_this_value = 5;
	xft.insert(3, insert_this_value);
	REQUIRE(xft.contains(3)); // Confirm key 3 exists

	// Remove key 3
	xft.remove(3);
	REQUIRE(xft.contains(3) == false); // Confirm key 3 no longer exists
}

TEST_CASE("[TRIES] Trie edge-case removal of last element", "[tries][last-removal]") {
	// New Trie instance sized for keys up to 1
	XFastTrie<int> xft(1);

	// Collect and insert variables into Trie
	int insert_this_value = 9;
	xft.insert(1, insert_this_value);

	// Confirm insertion 1 = 67
	REQUIRE(xft.contains(1));

	// Remove key 1
	xft.remove(1);

	// Confirm that key 1 no longer exists
	REQUIRE(xft.contains(1) == false);

	// Verify that the Trie is now empty where begin == end
	REQUIRE(xft.begin() == xft.end());
}

// =============================== [ UMAP ] ===============================

// =============================== [ RADIX ] ===============================
