#include "mat.h"

#include <print>
#include <random>
#include <utility>


int32_mat make_random_int32_mat(value_t lower_bound, value_t upper_bound) {
    thread_local std::random_device rd; 
    thread_local std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> distrib(lower_bound, upper_bound); 

    int32_mat ret;
    for (std::size_t y = 0; y < DIM; ++y) 
        for (std::size_t x = 0; x < DIM; ++x) 
            ret[y].row[x] = distrib(gen);

    return ret;
}

void print_mat(const int32_mat& mat) {
    for (std::size_t y = 0; y < DIM; ++y) {
        for (std::size_t x = 0; x < DIM; ++x) {
            std::print("{:8}", mat[y].row[x]); 
        }
        std::println();
    }
}

int32_mat transpose(const int32_mat& mat) {
    int32_mat ret;
    for (std::size_t y = 0; y < DIM; ++y) {
        for (std::size_t x = 0; x <= y; ++x) {
            ret[x].row[y] = mat[y].row[x];
            ret[y].row[x] = mat[x].row[y];
        }
    }
    return ret;
}


int32_mat mul_naive(const int32_mat& a, const int32_mat& b) {
    int32_mat ret;

    for (std::size_t y = 0; y < DIM; ++y) {
        for (std::size_t x = 0; x < DIM; ++x) {
            ret[y].row[x] = 0;
            for (std::size_t k = 0; k < DIM; ++k) {
                ret[y].row[x] += a[y].row[k] * b[k].row[x];
            }
        }
    }

    return ret;
}


int32_mat mul_simd(const int32_mat& a, const int32_mat& b) {
    int32_mat ret;

    auto b_transpose = transpose(b);

    stdx::native_simd<value_t> a_vector;
    stdx::native_simd<value_t> b_vector;
    std::size_t N = stdx::native_simd<value_t>::size();

    for (std::size_t y = 0; y < DIM; ++y) {
        for (std::size_t x = 0; x < DIM; ++x) {
            simd_t vsum(0);
            std::size_t k = 0;

            for (; k + W <= DIM; k += W) {
                simd_t va;
                simd_t vb;

                va.copy_from(&a[y].row[k], stdx::vector_aligned);
                vb.copy_from(&b_transpose[x].row[k], stdx::vector_aligned);

                vsum += va * vb;
            }


            value_t sum = 0;
            for (std::size_t lane = 0; lane < W; lane++)
                sum += vsum[lane];

            ret[y].row[x] = sum;
        }
    }

    return ret;
}
