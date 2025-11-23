#pragma once

#include <memory>
#include <stdexcept>

namespace jl {

template<typename T, size_t capacity>
class array {
private:

    std::unique_ptr<T[]> managed_ptr_;

public:

    array() : managed_ptr_(std::make_unique<T[]>(capacity)) {}

    // array(const array&);
    // array& operator=(const array&);

    // array(array&&);
    // array& operator=(array&&);

    // ELEMENT ACCESS

    T& at(size_t pos) {
        if (pos >= capacity) throw std::runtime_error("accessing out of range");
        return managed_ptr_[pos];
    }

    const T& at(size_t pos) const {
        if (pos >= capacity) throw std::runtime_error("accessing out of range");
        return managed_ptr_[pos];
    }

    T& operator[](size_t pos) {
        if (pos >= capacity) throw std::runtime_error("accessing out of range");
        return managed_ptr_[pos];
    }

    const T& operator[](size_t pos) const {
        if (pos >= capacity) throw std::runtime_error("accessing out of range");
        return managed_ptr_[pos];
    }

    T& front() {
        return managed_ptr_[0];
    }

    const T& front() const {
        return managed_ptr_[0];
    }

    T& back() {
        return managed_ptr_[capacity - 1];
    }

    const T& back() const {
        return managed_ptr_[capacity - 1];
    }

    // CAPACITY

    constexpr bool empty() const {
        return capacity == 0;
    }

    constexpr size_t size() const {
        return capacity;
    }

    constexpr size_t max_size() const {
        return capacity;
    }

};

}
