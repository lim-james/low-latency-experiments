#pragma once

#include <cstddef>
#include <stdexcept>
#include <print>
#include <utility>

namespace jl {

template<typename T>
class vector {
private:

    T* managed_ptr_       = nullptr;
    std::size_t size_     = 0;
    std::size_t capacity_ = 0;

public:

    vector() {}

    vector(std::size_t size) : size_(size), capacity_(size) {
        managed_ptr_ = new T[size_];
    }

    ~vector() {
        std::println("time to cleanup...");
        if (managed_ptr_ == nullptr)
            return;

        for (size_t i = 0; i < size_; ++i) 
            managed_ptr_[i].~T();

        ::operator delete[](managed_ptr_);
    }

    vector(const vector& to_be_copied) {
    }

    vector(vector&& to_be_moved) {

    }

    vector& operator=(const vector& to_be_copied) {
    }

    vector& operator=(vector&& to_be_moved) {
    }

    void reserve(std::size_t desired_capacity) {
        if (desired_capacity == 0 || capacity_ >= desired_capacity)
            return;

        std::size_t old_capacity_ = capacity_;
        capacity_ = desired_capacity;

        T* new_mem_space = static_cast<T*>(::operator new[](sizeof(T) * capacity_));
        std::println("moving to new mem region...");
        for (std::size_t i = 0; i < size_; ++i) 
            new_mem_space[i] = std::move(managed_ptr_[i]);
        std::println("completed move...");

        if (managed_ptr_ != nullptr && old_capacity_ > 0)
            ::operator delete[](managed_ptr_, sizeof(T) * old_capacity_);
        managed_ptr_ = new_mem_space;
    }

    void push_back(const T& value) {
        std::println(".push_back()...");
        if (size_ == capacity_) {
            expand_capacity();
        }

        new(managed_ptr_ + size_) T{value};
        size_++;
    }

    void push_back(T&& value) {
        std::println(".push_back()...");
        if (size_ == capacity_) {
            expand_capacity();
        }

        new(managed_ptr_ + size_) T{std::move(value)};
        size_++;
    }


    template<typename ...Args>
    void emplace_back(Args&&... args) {
        std::println(".emplace_back({})...", args...);
        if (size_ == capacity_) {
            expand_capacity();
        }

        new(managed_ptr_ + size_) T{std::forward<Args>(args)...};
        size_++;
    }

    void erase(std::size_t at) {
        if (at >= size_) {
            throw std::runtime_error("ERASING OUTSIDE OF VECTOR");
        }

        --size_;
        managed_ptr_[at].~T();
        for (std::size_t i = at; i < size_; ++i) {
            managed_ptr_[i] = std::move(managed_ptr_[i+1]);
        }
    }

    std::size_t size() const {
        return size_;    
    }

    std::size_t capacity() const {
        return capacity_;
    }

    void shrink_to_fit() {
    }

    T& operator[](std::size_t index) {
        return managed_ptr_[index]; 
    }

private:

    void expand_capacity() {
        std::println("requested to expand [size: {}, cap: {}]...", size_, capacity_);
        if (capacity_ == SIZE_MAX) {
            throw std::runtime_error("VECTOR HIT MAX CAPACITY");
        } else if (capacity_ >= SIZE_MAX >> 1) {
            reserve(SIZE_MAX);
        } else if (capacity_ == 0) {
            reserve(1);
        } else {
            reserve(capacity_ << 1);
        }
    }
        
};

}

