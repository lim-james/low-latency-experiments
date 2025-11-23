#include <array>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <print>
#include <chrono>
#include <ratio>
#include <cassert>
#include <ranges>
#include <algorithm>
#include <utility>
#include <random>
#include <format>

class [[nodiscard]] ScopeTimer {
private:
    std::string name_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;

public:
    ScopeTimer(std::string name)
        : name_(name)
        , start_(std::chrono::high_resolution_clock::now()) {}

    ~ScopeTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double, std::milli>(end - start_).count();
        std::println("{}: {}ms", name_, (int)duration);
    }
};


std::uint32_t parse_uint_naive(const char* ptr) { 
    std::uint32_t result = 0;
    while (*ptr) result = result * 10 + (*ptr++ - '0');
    return result;
}

constexpr std::uint64_t char_mask(std::uint8_t characters) {
    std::uint64_t mask = 0;
    for (std::uint8_t i = 0; i < characters; i++)
        mask = mask << 8 | 0xFF;
    return mask;
}

std::uint32_t parse_uint_simd(const char* ptr) { 
    if (!ptr[7]) [[likely]] {
        std::uint32_t result = 0;
        while (*ptr) result = result * 10 + (*ptr++ - '0');
        return result;
    }

    // std::uint64_t chunk;
    // memcpy(&chunk, ptr, 8);

    std::uint64_t chunk = *reinterpret_cast<const std::uint64_t*>(ptr);

    if ((chunk & 0xFF00000000000000ULL) != 0) {
        chunk = chunk - 0x3030303030303030ULL; 
        chunk = (chunk * 10    + (chunk >> 8))  & 0x00FF00FF00FF00FFULL;
        chunk = (chunk * 100   + (chunk >> 16)) & 0x0000FFFF0000FFFFULL;
        chunk = (chunk * 10000 + (chunk >> 32)) & 0x00000000FFFFFFFFULL;
        ptr += 8;
    }

    while (*ptr) chunk = chunk * 10 + (*ptr++ - '0');

    return chunk;
}

// template<std::uint8_t N>
// std::uint32_t parse_uint_simd_comp(const char (&ptr)[N]) { 
//     static_assert(1 <= N && N <= 8);
// 
//     // std::uint64_t chunk = 0;
//     // std::memcpy(&chunk, ptr, len);
//     constexpr std::uint64_t mask = (1ULL << (N << 3)) - 1;
//     constexpr std::uint64_t zero_mask = 0x3030303030303030ULL & mask;
// 
//     std::uint64_t chunk = (
//         (*reinterpret_cast<const std::uint64_t*>(ptr) & mask) 
//         - zero_mask
//     );
// 
//     chunk = (chunk * 10    + (chunk >> 8))  & 0x00FF00FF00FF00FFULL;
//     chunk = (chunk * 100   + (chunk >> 16)) & 0x0000FFFF0000FFFFULL;
//     chunk = (chunk * 10000 + (chunk >> 32)) & 0x00000000FFFFFFFFULL;
// 
//     static constexpr std::uint32_t pow10[9] = {
//         1u,
//         10u,
//         100u,
//         1000u,
//         10000u,
//         100000u,
//         1000000u,
//         10000000u,
//         100000000u
//     };
//     constexpr std::uint32_t scale = pow10[8 - N];
// 
//     return static_cast<std::uint32_t>(chunk / scale);
// }

std::uint32_t parse_uint_friendly(const char* ptr) { 
    std::uint32_t result = 0;
    while (*ptr) [[likely]] result = result * 10 + (*ptr++ - '0');
    return result;
}

int main() {
    std::uint32_t x;
    {
        x = std::stol("4294967294");
    }

    constexpr std::size_t ITERATIONS  = 10'000'000;

    constexpr std::array<std::uint32_t, 4> bounds{
        10'000,         // 4 chars
        10'000'000,     // 7 chars
        4'294'967'294   // >= 8 chars
    };

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> lower_distribution(0U, 9'999U);
    std::uniform_int_distribution<uint32_t> middle_distribution(10'000U, 9'999'999U);
    std::uniform_int_distribution<uint32_t> fixed_len_distribution(1'000U, 9'999U);
    std::uniform_int_distribution<uint32_t> upper_distribution(10'000'000U, 4'294'967'294U);

    auto make_str_int_pair = std::views::transform([](auto i) {
        return std::make_pair(std::to_string(i), i);
    });

    auto make_range = [&](size_t n, auto& dist) {
        return std::views::iota(0u, static_cast<uint32_t>(n))
             | std::views::transform([&](auto) { return dist(gen); })
             | make_str_int_pair;
    };

    auto variable_size_sets = {
        // std::pair{"FIXED  ",  make_range(ITERATIONS, fixed_len_distribution)},
        std::pair{"LOWER  ",  make_range(ITERATIONS, lower_distribution)},
        std::pair{"MIDDLE ", make_range(ITERATIONS, middle_distribution)},
        std::pair{"UPPER  ",  make_range(ITERATIONS, upper_distribution)}
    };

    for (const auto& [name, set]: variable_size_sets) {
        {
            auto _ = ScopeTimer(std::format("std::stoi // {}", name));
            for (const auto& [str, i]: set) 
                assert(i == std::stol(str));
        }
        {
            auto _ = ScopeTimer(std::format("naive     // {}", name));
            for (const auto& [str, i]: set) 
                assert(i == parse_uint_friendly(str.c_str()));
        }
        {
            auto _ = ScopeTimer(std::format("simd      // {}", name));
            for (const auto& [str, i]: set) 
            assert(i == parse_uint_simd(str.c_str()));
        }
        std::println();
        // {
        //     auto _ = ScopeTimer(std::format("simd comp // {}", name));
        //     for (const auto& [str, i]: set) 
        //     assert(i == parse_uint_simd_comp(str.c_str()));
        // }
    }

    return 0;
}
