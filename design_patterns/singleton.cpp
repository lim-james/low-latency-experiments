#include <print>
#include <cassert>
#include <type_traits>
#include <thread>
#include <array>

class Singleton {
private:
    Singleton() {
        std::println("CTOR");
    }

public:

    virtual ~Singleton() { 
        std::println("DTOR");
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

    [[nodiscard]] static Singleton& get_instance() {
        static Singleton instance;
        return instance;
    }

    bool operator==(const Singleton& other) const {
        return this == &other;
    }
};

int main() {
    Singleton& original_instance = Singleton::get_instance();

    assert(Singleton::get_instance() == original_instance);

    static_assert(!std::is_constructible<Singleton>(),         "Singleton shouldn't be constructible.");
    static_assert(!std::is_default_constructible<Singleton>(), "Singleton shouldn't be default constructible.");
    static_assert(!std::is_copy_constructible<Singleton>(),    "Singleton shouldn't be copy constructible.");
    static_assert(!std::is_move_constructible<Singleton>(),    "Singleton shouldn't be move constructible.");
    static_assert(!std::is_copy_assignable<Singleton>(),       "Singleton shouldn't be copy assignable.");
    static_assert(!std::is_move_assignable<Singleton>(),       "Singleton shouldn't be move assignable.");


    constexpr int MULTITHREAD_COUNT = 1000;

    {
        std::array<std::jthread, MULTITHREAD_COUNT> threads;

        for (int i = 0; i < MULTITHREAD_COUNT; ++i) {
            threads[i] = std::jthread([&original_instance]() { 
                assert(Singleton::get_instance() == original_instance);
            });
        }
    }
    

    return 0;
}
