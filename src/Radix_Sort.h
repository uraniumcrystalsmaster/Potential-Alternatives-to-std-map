#ifndef RADIX_SORT_H
#define RADIX_SORT_H

#include <vector>
#include <iterator>
#include <type_traits>

// Getter can be a lambda, functor, or any object with overloaded operator()
template <typename RandomAccessIt, typename Getter>
void radix_sort(RandomAccessIt begin, RandomAccessIt end, Getter get_value) {
    // Compile-time check: Requires Random Access for square bracket overload indexing and O(1) distance.
    static_assert(
        std::is_base_of<
            std::random_access_iterator_tag,
            typename std::iterator_traits<RandomAccessIt>::iterator_category
        >::value,
        "Radix sort optimization requires at least a random access iterator."
    );

    size_t range_size = std::distance(begin, end);
    if (range_size <= 1) return;

    using Key = std::decay_t<decltype(get_value(*begin))>;
    using Value = typename std::iterator_traits<RandomAccessIt>::value_type;

    using AnySignedInt = decltype(get_value(*begin));
    using AnyUnsignedInt = std::make_unsigned_t<Key>;

    constexpr AnyUnsignedInt SIGN_BIT_MASK = std::is_signed<Key>::value
        ? (AnyUnsignedInt(1) << (sizeof(Key) * 8 - 1))
        : 0;

    // Convert signed int to unsigned int for correct sorting
    auto get_converted_value = [&](const Value& item) {
        AnySignedInt signed_val = get_value(item);
        return static_cast<AnyUnsignedInt>(signed_val) ^ SIGN_BIT_MASK;
    };

    std::vector<Value> buffer(range_size);

    auto* src = &(*begin);
    auto* dest = buffer.data();

    constexpr int num_passes = sizeof(AnySignedInt);

    for (int pass = 0; pass < num_passes; ++pass) {
        size_t count[256] = {0};
        int shift = pass * 8;

        for (size_t i = 0; i < range_size; ++i) {
            AnyUnsignedInt val = get_converted_value(src[i]);
            int digit = (val >> shift) & 0xFF;
            ++count[digit];
        }

        for (int i = 1; i < 256; i++) {
            count[i] += count[i - 1];
        }

        for (size_t i = range_size; i > 0; --i) {
            AnyUnsignedInt val = get_converted_value(src[i - 1]);
            int digit = (val >> shift) & 0xFF;

            dest[count[digit] - 1] = std::move(src[i - 1]);
            --count[digit];
        }

        std::swap(src, dest);
    }

    if (num_passes % 2 != 0) {
        std::copy(src, src + range_size, dest);
    }
}

template <typename RandomAccessIt>
void radix_sort(RandomAccessIt begin, RandomAccessIt end) {
    static_assert(
        std::is_base_of<
            std::random_access_iterator_tag,
            typename std::iterator_traits<RandomAccessIt>::iterator_category
        >::value,
        "Radix sort optimization requires at least a random access iterator."
    );

    using Value = typename std::iterator_traits<RandomAccessIt>::value_type;

    static_assert(
        std::is_integral<Value>::value,
        "Radix sort without getter requires Value to be an integral type. "
        "For pairs or custom types, provide a getter function."
    );

    radix_sort(begin, end, [](const Value& value) { return value; });
}

// ---Doubly linked list overloads---

template <typename List, typename Getter>
void radix_sort(List& list, Getter get_value) {
    // Accessing internal types from the list
    using Node = typename List::Node;
    using Value = decltype(list.begin().operator*());
    using Key = std::decay_t<decltype(get_value(std::declval<Value>()))>;

    // Don't sort if empty or single element
    if (list.head == nullptr or list.head == list.tail) return;

    using AnyUnsignedInt = std::make_unsigned_t<Key>;

    constexpr AnyUnsignedInt SIGN_BIT_MASK = std::is_signed<Key>::value
        ? (AnyUnsignedInt(1) << (sizeof(Key) * 8 - 1))
        : 0;

    auto get_converted_value = [&](const Value& item) {
        Key signed_val = get_value(item);
        return static_cast<AnyUnsignedInt>(signed_val) ^ SIGN_BIT_MASK;
    };

    constexpr int num_passes = sizeof(Key);

    // Buckets for 256 radices (Head and Tail for O(1) append)
    Node* bucket_heads[256];
    Node* bucket_tails[256];

    for (int pass = 0; pass < num_passes; ++pass) {
        // Reset buckets
        for(int i = 0; i < 256; ++i) {
            bucket_heads[i] = nullptr;
            bucket_tails[i] = nullptr;
        }

        int shift = pass * 8;

        // 1. Distribute nodes into buckets
        Node* curr_node = list.head;
        while (curr_node) {
            Node* next_node = curr_node->next;

            // Calculate bucket index
            AnyUnsignedInt val = get_converted_value(curr_node->value);
            int digit = (val >> shift) & 0xFF;

            // Isolate current node
            curr_node->next = nullptr;
            curr_node->prev = nullptr;

            // Append to bucket
            if (bucket_tails[digit] == nullptr) {
                bucket_heads[digit] = curr_node;
                bucket_tails[digit] = curr_node;
            }
            else {
                bucket_tails[digit]->next = curr_node;
                curr_node->prev = bucket_tails[digit];
                bucket_tails[digit] = curr_node;
            }

            curr_node = next_node;
        }

        // 2. Reassemble list from buckets (Stitch)
        list.head = nullptr;
        list.tail = nullptr;

        for (int i = 0; i < 256; ++i) {
            if (bucket_heads[i] != nullptr) {
                if (list.head == nullptr) {
                    list.head = bucket_heads[i];
                    list.tail = bucket_tails[i];
                }
                else {
                    list.tail->next = bucket_heads[i];
                    bucket_heads[i]->prev = list.tail;
                    list.tail = bucket_tails[i];
                }
            }
        }

        // Ensure the new list tail ends correctly
        if (list.tail) {
            list.tail->next = nullptr;
        }
    }
}

template <typename List>
void radix_sort(List& list) {
    using Value = typename std::iterator_traits<typename List::iterator>::value_type;
    radix_sort(list, [](const Value& val) { return val; });
}

#endif //RADIX_SORT_H