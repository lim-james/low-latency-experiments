#pragma once

#include <array>
#include <random>
#include <print>

#include <experimental/simd>

namespace stdx = std::experimental;

template<typename T, std::size_t N>
class alignas(stdx::memory_alignment_v<stdx::native_simd<T>>) SquareMatrix {
private:

    using simd_t = stdx::native_simd<T>;

    std::array<T, N*N> matrix_;
    std::array<T, N*N> transposed_;

    constexpr static inline std::size_t getIndex(std::size_t x, std::size_t y) {
        return y * N + x;
    }

public:

    static SquareMatrix make_random(T lower_bound, T upper_bound) {
        thread_local std::random_device rd; 
        thread_local std::mt19937 gen(rd()); 
        std::uniform_int_distribution<> distrib(lower_bound, upper_bound); 

        SquareMatrix random_matrix;
        for (std::size_t i = 0; i < N*N; ++i) 
            random_matrix.matrix_[i] = distrib(gen);

        for (std::size_t y = 0; y < N; ++y) {
            for (std::size_t x = 0; x <= y; ++x) {
                random_matrix.transposed_[getIndex(x,y)] = random_matrix.matrix_[getIndex(y,x)];
                random_matrix.transposed_[getIndex(y,x)] = random_matrix.matrix_[getIndex(x,y)];
            }
        }

        return random_matrix;
    }

    void print() {
        for (std::size_t y = 0; y < N; ++y) {
            for (std::size_t x = 0; x < N; ++x) {
                std::print("{:8} ", matrix_[getIndex(x,y)]); 
            }
            std::println();
        }
    }

    SquareMatrix mul_naive(const SquareMatrix& other) const {
        SquareMatrix product;

        for (std::size_t y = 0; y < N; ++y) {
            for (std::size_t x = 0; x < N; ++x) {
                product.matrix_[getIndex(x,y)] = 0;
                for (std::size_t k = 0; k < N; ++k) {
                    product.matrix_[getIndex(x,y)] += matrix_[getIndex(k,y)] * other.matrix_[getIndex(x,k)];
                }
            }
        }

        return product;
    }


    SquareMatrix mul_simd(const SquareMatrix& other) const {
        SquareMatrix product;

        simd_t a_vector;
        simd_t b_vector;

        for (std::size_t y = 0; y < N; ++y) {
            for (std::size_t x = 0; x < N; ++x) {
                simd_t vsum(0);
                std::size_t k = 0;

                auto a_row = matrix_.data() + y * N;
                auto b_col = other.transposed_.data() + x * N;

                for (; k + simd_t::size() <= N; k += simd_t::size()) {
                    simd_t va;
                    simd_t vb;

                    va.copy_from(a_row + k, stdx::vector_aligned);
                    vb.copy_from(b_col + k, stdx::vector_aligned);

                    vsum += va * vb;
                }


                //T sum = 0;
                //for (std::size_t lane = 0; lane < simd_t::size(); lane++)
                //    sum += vsum[lane];

                //product.matrix_[getIndex(x,y)] = sum;
                product.matrix_[getIndex(x,y)] = vsum[0] + vsum[1] + vsum[2] + vsum[3];
            }
        }

        return product;
    }

    bool operator==(const SquareMatrix& other) const {
        for (std::size_t i = 0; i < N*N; ++i)
            if (matrix_[i] != other.matrix_[i])
                return false;
        return true;
    }
};

