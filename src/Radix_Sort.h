#ifndef RADIX_SORT_H
#define RADIX_SORT_H

#include <vector>
#include <iterator>
#include <type_traits>

// Getter can be a lambda, functor, or any object with overloaded operator()
template <typename BidirectionalIt, typename Getter>
void radix_sort(BidirectionalIt begin, BidirectionalIt end, Getter get_value) {
    // Compile-time check for iterator type
    static_assert(
        std::is_base_of<
            std::bidirectional_iterator_tag,
            typename std::iterator_traits<BidirectionalIt>::iterator_category
        >::value,
        "Radix sort requires at least a bidirectional iterator."
    );

    auto range_size = std::distance(begin, end);
    if (range_size <= 1) return;

    using Value = typename std::iterator_traits<BidirectionalIt>::value_type;
    using AnySignedInt = decltype(get_value(*begin));
    using AnyUnsignedInt = std::make_unsigned_t<AnySignedInt>;

    constexpr AnyUnsignedInt SIGN_BIT_MASK = AnyUnsignedInt(1) << (sizeof(AnySignedInt) * 8 - 1);

    // Convert signed int to unsigned int
    auto get_converted_value = [&](const Value& item) {
        AnySignedInt signed_val = get_value(item);
        return (AnyUnsignedInt)(signed_val) ^ SIGN_BIT_MASK; // Cast to unsigned, then flip the sign bit
    };

    std::vector<Value> output(range_size);

    constexpr int num_passes = sizeof(AnySignedInt);

    for (int pass = 0; pass < num_passes; ++pass) {
        size_t count[256] = {0};
        int shift = pass * 8;

        // 1. Count instances of each byte
        for (BidirectionalIt it = begin; it != end; ++it) {
            AnyUnsignedInt val = get_converted_value(*it);
            // Get the 'pass'-th byte
            int digit = (val >> shift) & 0xFF;
            ++count[digit];
        }

        // 2. Total count (prefix sum)
        for (int i = 1; i < 256; i++) {
            count[i] += count[i - 1];
        }

        // 3. Build output array (backward pass)
        using ReverseIt = std::reverse_iterator<BidirectionalIt>;
        ReverseIt r_begin(end);
        ReverseIt r_end(begin);

        for (ReverseIt it = r_begin; it != r_end; ++it) {
            AnyUnsignedInt val = get_converted_value(*it);
            int digit = (val >> shift) & 0xFF;

            output[count[digit] - 1] = *it;
            --count[digit];
        }

        // 4. Copy back
        std::copy(output.begin(), output.end(), begin);
    }
}

template <typename BidirectionalIt>
void radix_sort(BidirectionalIt begin, BidirectionalIt end) {
    static_assert(
        std::is_base_of<
            std::bidirectional_iterator_tag,
            typename std::iterator_traits<BidirectionalIt>::iterator_category
        >::value,
        "Radix sort requires at least a bidirectional iterator."
    );

    using Value = typename std::iterator_traits<BidirectionalIt>::value_type;

    static_assert(
        std::is_integral<Value>::value,
        "Radix sort without getter requires Value to be an integral type. "
        "For pairs or custom types, provide a getter function."
    );

    radix_sort(begin, end, [](const Value& value) { return value; });
}

#endif //RADIX_SORT_H