#include <atomic>
#include <utility>

class Copyable {
    std::atomic<int> a_;
public:
    Copyable(int a): a_(a) {}
};

int main() {
    Copyable og{15};
    
    std::atomic<int> a{1};
    // std::atomic<int> b{a};
    // std::atomic<int> b{std::move(a)};

    return 0;
}
