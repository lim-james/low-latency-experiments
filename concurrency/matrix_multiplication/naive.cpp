#include "mat.h"

#include <print>
#include <chrono>
#include <cassert>

template<typename ...Args>
inline void log_row(Args... args) {
    std::println("{:5} | {:4} | {:7} | {:7}", args...);
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

template<std::size_t SCALE, int ITERATIONS>
void test() {
    constexpr std::size_t DIM = stdx::native_simd<std::int32_t>::size() * SCALE;

    auto a = SquareMatrix<std::int32_t, DIM>::make_random(1, 10);
    auto b = SquareMatrix<std::int32_t, DIM>::make_random(1, 10);

    SquareMatrix<std::int32_t, DIM> naive_result;
    SquareMatrix<std::int32_t, DIM> simd_result;

    double naive_tottime;
    double simd_tottime;
    
    {
        auto _ = ScopeTimer(&naive_tottime);
        for (int i = 0; i < ITERATIONS; ++i) 
            naive_result = a.mul_naive(b);

    }

    {
        auto _ = ScopeTimer(&simd_tottime);
        for (int i = 0; i < ITERATIONS; ++i) 
            simd_result = a.mul_simd(b);
    }

    const int naive_throughput = std::round(calculate_throughput_per_s(naive_tottime, ITERATIONS));
    const int simd_throughput  = std::round(calculate_throughput_per_s(simd_tottime,  ITERATIONS));

    log_row(ITERATIONS, DIM, naive_throughput, simd_throughput);

    assert(naive_result == simd_result);
}

template<
    std::pair<std::size_t,std::size_t> SCALE, 
    std::pair<std::size_t,std::size_t> ITERATIONS
>
constexpr void test_recursive() {
    constexpr auto LOWER_SCALE = SCALE.first;
    constexpr auto UPPER_SCALE = SCALE.second;

    constexpr auto LOWER_ITERATIONS = ITERATIONS.first;
    constexpr auto UPPER_ITERATIONS = ITERATIONS.second;

    test<LOWER_SCALE, LOWER_ITERATIONS>();

    if constexpr (LOWER_SCALE < UPPER_SCALE) 
        test_recursive<{LOWER_SCALE*2, UPPER_SCALE}, ITERATIONS>();
    if constexpr (LOWER_ITERATIONS < UPPER_ITERATIONS) 
        test_recursive<SCALE, {LOWER_ITERATIONS*10,UPPER_ITERATIONS}>();
}

int main() {
    log_row("COUNT", "SIZE", "NAIVE", "SIMD");
    std::println("--------------------------------");
    test_recursive<{16,64}, {100,100}>();
    return 0;
}
