#include <vector>
#include <array>
#include <print>
#include <cstddef>
#include <chrono>
#include <memory>

class [[nodiscard]] ScopeTimer {
private:
    std::string name_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;

public:
    ScopeTimer(std::string name)
        : name_(name)
        , start_(std::chrono::high_resolution_clock::now()) {}

    ~ScopeTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double, std::milli>(end - start_).count();
        std::println("{}: {}ms", name_, (int)duration);
    }
};

template<typename T>
class MemAudit {
private:
    static std::size_t instance_count;
    static std::size_t copy_count;
    static std::size_t move_count;

    std::size_t id_ = 0;
    T value_{};

public:
    MemAudit() : id_(++instance_count) {
        // std::println("  [MEM AUDIT] CTOR #{}", id_);
    }

    MemAudit(const T& value) 
        : id_(++instance_count) 
        , value_(value) {
        // std::println("  [MEM AUDIT] CTOR #{} = {}", id_, value_);
    }

    virtual ~MemAudit() {
        // std::println("  [MEM AUDIT] DTOR #{}", id_);
    }

    MemAudit(const MemAudit& other) {
        id_ = other.id_;
        value_ = other.value_;
        // std::println("  [MEM AUDIT] COPY #{} | {}", id_, ++copy_count);
    }

    MemAudit(MemAudit&& other) {
        id_ = other.id_;
        value_ = other.value_;
        // std::println("  [MEM AUDIT] MOVE #{} | {}", id_, ++move_count);
    }

    MemAudit<T>& operator=(const MemAudit& other) { 
        id_ = other.id_;
        value_ = other.value_;
        // std::println("  [MEM AUDIT] COPY #{} | {}", id_, ++copy_count);
        return *this;
    }

    MemAudit<T>& operator=(MemAudit&& other) {
        id_ = other.id_;
        value_ = other.value_;
        // std::println("  [MEM AUDIT] MOVE #{} | {}", id_, ++move_count);
        return *this;     
    }

    T get() const {
        // std::println("get {}", value_);
        return value_;
    }
};


template<typename T> std::size_t MemAudit<T>::instance_count = 0;
template<typename T> std::size_t MemAudit<T>::copy_count = 0;
template<typename T> std::size_t MemAudit<T>::move_count = 0;


template<typename T>
class CustomAllocator {
public:
    using pointer = T*;
    using value_type = T;
    using size_type = std::size_t;

    pointer allocate(size_type n) {
        // std::println("custom alloc {}", n);
        void* allocated_mem = ::operator new(n * sizeof(T));
        return static_cast<T*>(allocated_mem);
    }

    void deallocate(pointer p, size_type n) {
        // std::println("custom dealloc {}", n);
        ::operator delete(p);
    }
};

template<typename T, typename U>
bool operator==(const CustomAllocator<T>&, const CustomAllocator<U>&) {
    return true;
}

template<typename T, typename U>
bool operator!=(const CustomAllocator<T>&, const CustomAllocator<U>&) {
    return false;
}

template<typename T, std::size_t PoolSize>
class PoolAllocator {

public:

    using pointer = T*;
    using value_type = T;
    using size_type = std::size_t;

    template<typename U>
    struct rebind {
        using other = PoolAllocator<U, PoolSize>;
    };
     
    struct Buffer {
        alignas(alignof(T)) std::byte storage[PoolSize * sizeof(T)];
        size_type used = 0;
    };
    
    std::shared_ptr<Buffer> buffer_;
    
    PoolAllocator() : buffer_(std::make_shared<Buffer>()) {}
    
    PoolAllocator(const PoolAllocator& other) = default;
    
    template<typename U>
    PoolAllocator(const PoolAllocator<U, PoolSize>& other) 
        : buffer_(std::reinterpret_pointer_cast<Buffer>(other.buffer_)) {}
    
    pointer allocate(size_type n) {
        if (buffer_->used + n > PoolSize) [[unlikely]] {
            throw std::bad_alloc();
        }
        
        pointer result = reinterpret_cast<pointer>(
            &buffer_->storage[buffer_->used * sizeof(T)]
        );
        buffer_->used += n;
        
        return result;
    }
    
    void deallocate(pointer p, size_type n) noexcept {}
    
};

template<typename T, typename U, std::size_t PoolSize>
bool operator==(const PoolAllocator<T, PoolSize>& a, const PoolAllocator<U, PoolSize>& b) {
    return a.buffer_ == b.buffer_;
}

template<typename T, typename U, std::size_t PoolSize>
bool operator!=(const PoolAllocator<T, PoolSize>& a, const PoolAllocator<U, PoolSize>& b) {
    return !(a == b);
}

int main() {
    constexpr std::size_t item_count = 1 << 21;

    using obj_t = MemAudit<char>;
    {
        auto _ = ScopeTimer("No alloc");
        std::vector<obj_t>  vec;
        for (int i = 0; i < item_count; ++i)
            vec.emplace_back(i);
    }

    {
        auto _ = ScopeTimer("No alloc + reserve");
        std::vector<obj_t>  vec;
        vec.reserve(item_count);
        for (int i = 0; i < item_count; ++i)
            vec.emplace_back(i);
    }

    {
        auto _ = ScopeTimer("Custom alloc");
        std::vector<obj_t, CustomAllocator<obj_t>>  vec;
        for (int i = 0; i < item_count; ++i)
            vec.emplace_back(i);
    }

    {
        auto _ = ScopeTimer("stack alloc");
        std::vector<obj_t, PoolAllocator<obj_t, item_count << 1>>  vec;
        for (int i = 0; i < item_count; ++i)
            vec.emplace_back(i);
    }
    return 0;
}
