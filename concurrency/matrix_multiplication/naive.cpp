#include "mat.h"

#include <print>
#include <chrono>
#include <cassert>

template<typename ...Args>
inline void log_row(Args... args) {
    std::println("{:5} | {:4} | {:10} | {:10} | {:5}", args...);
}

class [[nodiscard]] ScopeTimer {
private:
    double* out_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;

public:
    ScopeTimer(double* out)
        : out_(out)
        , start_(std::chrono::high_resolution_clock::now()) {}

    ~ScopeTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        *out_ = std::chrono::duration<double, std::milli>(end - start_).count();
    }
};


constexpr double calculate_throughput_per_s(double tottime_ms, std::size_t ncalls) {
    constexpr double ms_to_s = 1'000.0;
    return static_cast<double>(ncalls) / tottime_ms * ms_to_s; 
}

constexpr double calculate_throughput_per_ms(double tottime_ms, std::size_t ncalls) {
    return static_cast<double>(ncalls) / tottime_ms; 

}

template<std::size_t DIM, std::size_t ITERATIONS>
void test() {
    auto a = SquareMatrix<std::int32_t, DIM>::make_random(1, 10);
    auto b = SquareMatrix<std::int32_t, DIM>::make_random(1, 10);

    SquareMatrix<std::int32_t, DIM> naive_result;
    SquareMatrix<std::int32_t, DIM> simd_result;

    double naive_tottime;
    double simd_tottime;
    
    {
        auto _ = ScopeTimer(&naive_tottime);
        for (std::size_t i = 0; i < ITERATIONS; ++i) 
            naive_result = a.mul_naive(b);

    }

    {
        auto _ = ScopeTimer(&simd_tottime);
        for (std::size_t i = 0; i < ITERATIONS; ++i) 
            simd_result = a.mul_simd(b);
    }

    const int naive_throughput = std::round(calculate_throughput_per_s(naive_tottime, ITERATIONS));
    const int simd_throughput  = std::round(calculate_throughput_per_s(simd_tottime,  ITERATIONS));
    const double scale = std::round((naive_tottime / simd_tottime) * 100) / 100;

    log_row(ITERATIONS, DIM, naive_throughput, simd_throughput, scale);

    assert(naive_result == simd_result);
}

template<
    std::size_t SCALE, 
    std::size_t... ITERATIONS
>
constexpr void test_iterations() {
    (test<SCALE, ITERATIONS>(), ...);
}

int main() {
    log_row("COUNT", "SIZE", "NAIVE", "SIMD", "SCALE");
    std::println("----------------------------------------");
    test_iterations<4,   10'000>();
    test_iterations<8,   10'000>();
    test_iterations<16,  10'000>();
    test_iterations<32,  10'000>();
    test_iterations<64,  10'000>();
    test_iterations<128, 10'000>();
    test_iterations<256, 10'000>();
    return 0;
}
