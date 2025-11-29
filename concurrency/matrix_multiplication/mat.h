#pragma once

#include <array>
#include <cstdint>
#include <experimental/simd>

namespace stdx = std::experimental;

using value_t = std::int32_t;
using simd_t  = stdx::native_simd<value_t>;

constexpr std::size_t W = stdx::native_simd<value_t>::size();
constexpr std::size_t DIM = W * 64;

struct alignas(stdx::memory_alignment_v<simd_t>) aligned_row_t {
    std::array<value_t, DIM> row;
};
using int32_mat     = std::array<aligned_row_t, DIM>;

int32_mat make_random_int32_mat(value_t lower_bound, value_t upper_bound);

void print_mat(const int32_mat& mat);

int32_mat transpose(const int32_mat& mat);

int32_mat mul_naive(const int32_mat& a, const int32_mat& b);
int32_mat mul_simd(const int32_mat& a, const int32_mat& b);
