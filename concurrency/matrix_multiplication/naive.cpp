#include "mat.h"
#include <print>

#include <chrono>

class ScopeTimer {
private:
    const char* name_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;

public:
    ScopeTimer(const char* name)
        : name_(name)
        , start_(std::chrono::high_resolution_clock::now()) {}

    ~ScopeTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double>(end - start_).count();
        std::println("{}: {}", name_, duration);
    }
};

int main() {
    auto a = make_random_int32_mat(1, 100);
    auto b = make_random_int32_mat(1, 100);


    constexpr int ITERATIONS = 1;
    
    {
        auto _ = ScopeTimer("naive");
        for (int i = 0; i < ITERATIONS; ++i) 
            volatile auto res = mul_naive(a, b);
    }


    std::println();
    {
        auto _ = ScopeTimer("simd ");
        for (int i = 0; i < ITERATIONS; ++i) 
            volatile auto res = mul_simd(a, b);
    }

    return 0;
}
