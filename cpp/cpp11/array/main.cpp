#include "array.hpp"

#include <array>
#include <print> 

template<typename T>
class MemAudit {
private:
    static std::size_t instance_count;
    static std::size_t copy_count;
    static std::size_t move_count;

    std::size_t id_ = 0;
    T value_{};

public:
    MemAudit() : id_(++instance_count) {std::println("  [MEM AUDIT] CTOR #{}", id_);}

    MemAudit(const T& value) 
        : id_(++instance_count) 
        , value_(value) {
        std::println("  [MEM AUDIT] CTOR #{} = {}", id_, value_);
    }

    virtual ~MemAudit() {std::println("  [MEM AUDIT] DTOR #{}", id_);}

    MemAudit(const MemAudit& other) {
        id_ = other.id_;
        value_ = other.value_;
        std::println("  [MEM AUDIT] COPY #{} | {}", id_, ++copy_count);
    }

    MemAudit(MemAudit&& other) {
        id_ = other.id_;
        value_ = other.value_;
        std::println("  [MEM AUDIT] MOVE #{} | {}", id_, ++move_count);
    }

    MemAudit<T>& operator=(const MemAudit& other) { 
        id_ = other.id_;
        value_ = other.value_;
        std::println("  [MEM AUDIT] COPY #{} | {}", id_, ++copy_count);
        return *this;
    }

    MemAudit<T>& operator=(MemAudit&& other) {
        id_ = other.id_;
        value_ = other.value_;
        std::println("  [MEM AUDIT] MOVE #{} | {}", id_, ++move_count);
        return *this;     
    }

    T get() const {
        std::println("get {}", value_);
        return value_;
    }
};


template<typename T> std::size_t MemAudit<T>::instance_count = 0;
template<typename T> std::size_t MemAudit<T>::copy_count = 0;
template<typename T> std::size_t MemAudit<T>::move_count = 0;

int main() {
    {
        jl::array<MemAudit<int>, 5> my_array;
        std::println("SIZE::{}", sizeof(my_array));
        my_array[0] = 5;
    }

    std::println("\n\n");
    {
        std::array<MemAudit<int>, 5> my_array;

    }

    return 0;
}
