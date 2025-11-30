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

    SquareMatrix()
        : matrix_{0}
        , transposed_{0} {}

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

    void print() const {
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
        SquareMatrix product{};

        constexpr std::size_t TILE_SIZE = 32;

        simd_t va, vb;

        const T* A = matrix_.data();
        const T* BT = other.transposed_.data();
        T* C = product.matrix_.data();

        for (std::size_t ii = 0; ii < N; ii += TILE_SIZE) {
            for (std::size_t jj = 0; jj < N; jj += TILE_SIZE) {
                for (std::size_t kk = 0; kk < N; kk += TILE_SIZE) {

                    const std::size_t i_end = std::min(ii + TILE_SIZE, N);
                    const std::size_t j_end = std::min(jj + TILE_SIZE, N);
                    const std::size_t k_end = std::min(kk + TILE_SIZE, N);

                    for (std::size_t i = ii; i < i_end; ++i) {
                        const T* a_row = A + i*N;

                        for (std::size_t j = jj; j < j_end; ++j) {
                            simd_t vsum(0);

                            const T* b_row = BT + j*N;

                            for (std::size_t k = kk; k + simd_t::size() <= k_end; k += simd_t::size()) {
                                va.copy_from(a_row + k, stdx::vector_aligned);
                                vb.copy_from(b_row + k, stdx::vector_aligned);
                                vsum += va * vb;
                            }

                            C[i*N + j] += vsum[0] + vsum[1] + vsum[2] + vsum[3];
                        }
                    }

                }
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

