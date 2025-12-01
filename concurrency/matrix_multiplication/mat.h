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
        constexpr std::size_t NR = 4;

        const T* A = matrix_.data();
        const T* BT = other.transposed_.data();
        T* C = product.matrix_.data();

        alignas(64) T A_pack[NR * TILE_SIZE];
        alignas(64) T B_pack[NR * TILE_SIZE];

        for (std::size_t ii = 0; ii < N; ii += TILE_SIZE) {
            for (std::size_t jj = 0; jj < N; jj += TILE_SIZE) {
                for (std::size_t kk = 0; kk < N; kk += TILE_SIZE) {
                    
                    const std::size_t i_end = std::min(ii + TILE_SIZE, N);
                    const std::size_t j_end = std::min(jj + TILE_SIZE, N);
                    const std::size_t k_end = std::min(kk + TILE_SIZE, N);

                    const std::size_t K_blk  = k_end - kk;

                    for (std::size_t i = ii; i < i_end; i += NR) {

                        const T* A0 = A + (i+0)*N + kk;
                        const T* A1 = A + (i+1)*N + kk;
                        const T* A2 = A + (i+2)*N + kk;
                        const T* A3 = A + (i+3)*N + kk;

                        pack_tiles(A0, A1, A2, A3, A_pack, K_blk);

                        for (std::size_t j = jj; j < j_end; j += NR) {
                            const T* B0 = BT + (j+0)*N + kk;
                            const T* B1 = BT + (j+1)*N + kk;
                            const T* B2 = BT + (j+2)*N + kk;
                            const T* B3 = BT + (j+3)*N + kk;

                            pack_tiles(B0, B1, B2, B3, B_pack, K_blk);
                                
                            T* C0 = C + (i+0)*N + j;
                            T* C1 = C + (i+1)*N + j;
                            T* C2 = C + (i+2)*N + j;
                            T* C3 = C + (i+3)*N + j;

                            microkernel_4x4(
                                A_pack, B_pack, 
                                C0, C1, C2, C3,
                                K_blk
                            );

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

private:

    void pack_tiles(
        const T* R0, const T* R1, const T* R2, const T* R3,
        T* pack, std::size_t K_blk
    ) const {
        for (std::size_t k = 0; k < K_blk; ++k) {
            pack[0*K_blk + k] = R0[k];
            pack[1*K_blk + k] = R1[k];
            pack[2*K_blk + k] = R2[k];
            pack[3*K_blk + k] = R3[k];
        }
    }

    void microkernel_4x4(
        const T* A_pack,
        const T* B_pack,
        T* C0, T* C1, T* C2, T* C3,
        std::size_t K_blk
    ) const {
        simd_t c00(0), c01(0), c02(0), c03(0);
        simd_t c10(0), c11(0), c12(0), c13(0);
        simd_t c20(0), c21(0), c22(0), c23(0);
        simd_t c30(0), c31(0), c32(0), c33(0);

        const std::size_t K_simd = (K_blk / simd_t::size()) * simd_t::size();
        for (std::size_t k = 0; k < K_simd; k += simd_t::size()) {
            simd_t a0, a1, a2, a3;
            a0.copy_from(A_pack + 0*K_blk + k, stdx::vector_aligned);
            a1.copy_from(A_pack + 1*K_blk + k, stdx::vector_aligned);
            a2.copy_from(A_pack + 2*K_blk + k, stdx::vector_aligned);
            a3.copy_from(A_pack + 3*K_blk + k, stdx::vector_aligned);

            simd_t b0, b1, b2, b3;
            b0.copy_from(B_pack + 0*K_blk + k, stdx::vector_aligned);
            b1.copy_from(B_pack + 1*K_blk + k, stdx::vector_aligned);
            b2.copy_from(B_pack + 2*K_blk + k, stdx::vector_aligned);
            b3.copy_from(B_pack + 3*K_blk + k, stdx::vector_aligned);

            c00 += a0 * b0; c01 += a0 * b1; c02 += a0 * b2; c03 += a0 * b3;
            c10 += a1 * b0; c11 += a1 * b1; c12 += a1 * b2; c13 += a1 * b3;
            c20 += a2 * b0; c21 += a2 * b1; c22 += a2 * b2; c23 += a2 * b3;
            c30 += a3 * b0; c31 += a3 * b1; c32 += a3 * b2; c33 += a3 * b3;
        }

        C0[0] += c00[0] + c00[1] + c00[2] + c00[3];
        C0[1] += c01[0] + c01[1] + c01[2] + c01[3];
        C0[2] += c02[0] + c02[1] + c02[2] + c02[3];
        C0[3] += c03[0] + c03[1] + c03[2] + c03[3];

        C1[0] += c10[0] + c10[1] + c10[2] + c10[3];
        C1[1] += c11[0] + c11[1] + c11[2] + c11[3];
        C1[2] += c12[0] + c12[1] + c12[2] + c12[3];
        C1[3] += c13[0] + c13[1] + c13[2] + c13[3];

        C2[0] += c20[0] + c20[1] + c20[2] + c20[3];
        C2[1] += c21[0] + c21[1] + c21[2] + c21[3];
        C2[2] += c22[0] + c22[1] + c22[2] + c22[3];
        C2[3] += c23[0] + c23[1] + c23[2] + c23[3];

        C3[0] += c30[0] + c30[1] + c30[2] + c30[3];
        C3[1] += c31[0] + c31[1] + c31[2] + c31[3];
        C3[2] += c32[0] + c32[1] + c32[2] + c32[3];
        C3[3] += c33[0] + c33[1] + c33[2] + c33[3];
    }
};

