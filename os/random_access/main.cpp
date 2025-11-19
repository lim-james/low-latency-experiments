#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include <functional>
#include <algorithm>
#include <unistd.h>
#include <array>

const size_t CACHE_LINE_SIZE = 128;

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
        std::cout << name_ << ": " << duration << '\n';
    }
};

class ManTimer {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;

public:
    ManTimer(): start_(std::chrono::high_resolution_clock::now()) {}

    double end() {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(end - start_).count();
    }
};


void flush() {
    const size_t BUFFER_SIZE = 256 * 1024 * 1024;  
    char* buffer = new char[BUFFER_SIZE];
    for (size_t i = 0; i < BUFFER_SIZE; i += CACHE_LINE_SIZE) 
        buffer[i] = 'a';
    delete[] buffer;
}


void iterate_read(char* buffer, size_t buffer_size, size_t stride) {
    size_t probe = 0;
    volatile char sink;
    for (size_t i = 0; i < buffer_size; ++i) {
        sink = buffer[probe]; // test  read
        probe = (probe + stride) % buffer_size;
    }
}


void iterate_write(char* buffer, size_t buffer_size, size_t stride) {
    size_t probe = 0;
    for (size_t i = 0; i < buffer_size; ++i) {
        buffer[probe] = static_cast<char>(probe); // test write
        probe = (probe + stride) % buffer_size;
    }
}


struct benchmark_times {
    double mean;
    double p99;
};


benchmark_times benchmark(const std::function<void()>& fn, size_t iterations) {
    double time_sum = 0.0;
    double p99 = 0.0;

    double t;

    for (size_t i=0; i<iterations; ++i) {
        flush();
        ManTimer timer;
        fn();
        t = timer.end();
        p99 = std::max(t, p99);
        time_sum += t;
    }

    return benchmark_times{time_sum / (double) iterations, p99};
}


void process_main(
    size_t buffer_size_mb,
    unsigned char id, 
    unsigned char process_max_exponent
) {
    const size_t BUFFER_SIZE = buffer_size_mb * 1024 * 1024;  
    char* buffer = static_cast<char*>(aligned_alloc(CACHE_LINE_SIZE, BUFFER_SIZE));

    for (unsigned char i = 0; i < process_max_exponent; ++i) {
        size_t stride = 1 << (id * process_max_exponent + i);

        auto[read_avg, read_p99] = benchmark(
            [&]() { iterate_read(buffer, BUFFER_SIZE, stride); },
            100
        );       

        auto[write_avg, write_p99] = benchmark(
            [&]() { iterate_write(buffer, BUFFER_SIZE, stride); },
            100
        );       

        double read_delta = read_p99 - read_avg;
        double write_delta = write_p99 - write_avg;
        
        std::cout << std::left
                  << std::setw(8) << stride
                  << " | "
                  << std::setw(7) << std::fixed << std::setprecision(3) << read_avg
                  << " | "
                  << std::setw(7) << std::fixed << std::setprecision(3) << read_p99
                  << " | "
                  << std::setw(8) << std::fixed << std::setprecision(3) << read_delta
                  << " | "
                  << std::setw(7) << std::fixed << std::setprecision(3) << write_avg
                  << " | "
                  << std::setw(7) << std::fixed << std::setprecision(3) << write_p99
                  << " | "
                  << std::setw(8) << std::fixed << std::setprecision(3) << write_delta
                  << '\n';
    }

    delete[] buffer;
}


void buffered_main(size_t buffer_size_mb) {
    std::cout << std::left
              << std::setw(8) << ""
              << " | "
              << std::setw(28) << "READ"
              << " | "
              << std::setw(7) << "WRITE"
              << '\n';

    std::cout << std::left
              << std::setw(8) << "STRIDE"
              << " | "
              << std::setw(7) << "P50"
              << " | "
              << std::setw(7) <<  "P99"
              << " | "
              << std::setw(8) << "DIFF"
              << " | "
              << std::setw(7) << "P50"
              << " | "
              << std::setw(7) <<  "P99"
              << " | "
              << std::setw(8) << "DIFF"
              << '\n';

    static const unsigned char MAX_EXPONENT = 24;
    static const unsigned char PROCESS_COUNT = 8;
    static const unsigned char PROCESS_MAX_EXPONENT = MAX_EXPONENT / PROCESS_COUNT;

    pid_t pid;

    for (unsigned char i = 1; i < PROCESS_COUNT; ++i) {
        pid = fork();

        if (pid == 0) {
            process_main(buffer_size_mb, i, PROCESS_MAX_EXPONENT);
            _exit(0);
        }
    }

    process_main(buffer_size_mb, 0, PROCESS_MAX_EXPONENT);

    for (unsigned char i = 1; i < PROCESS_COUNT; ++i) 
        wait(nullptr);
}


int main() {
    std::array<size_t, 3> buffer_sizes{16,32,64};

    for (auto b: buffer_sizes) {
        std::cout << "\n\n>>> TESTING BUFFER SIZE OF " << b << "MB\n";
        buffered_main(b);
    }

    return 0;
}
