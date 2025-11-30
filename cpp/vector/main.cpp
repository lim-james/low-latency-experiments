#include "vector.h"
#include <vector>

#include <print>
#include <cassert>

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
    std::println("start of program...");
    jl::vector<MemAudit<int>> v;
    v.reserve(10);

    for (int i = 0; i < 5; ++i) {
        v.emplace_back(i);
    }

    for (int i = 5; i < 10; ++i) {
        v.push_back(MemAudit<int>{i});
    }
    
    std::println("running tests...");
    for (int i = 0; i < v.size(); ++i) {
        assert(i == v[i].get());
    }

    std::size_t original_size = v.size();
    v.erase(5);  
    assert(v.size() == original_size - 1);

    for (int i = 5; i < v.size(); ++i) {
        assert(i + 1 == v[i].get());
    }

    std::println("looks good! goodbye...");

    return 0;
}
