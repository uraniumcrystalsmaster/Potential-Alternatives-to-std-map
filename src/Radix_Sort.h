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

#endif //RADIX_SORT_H