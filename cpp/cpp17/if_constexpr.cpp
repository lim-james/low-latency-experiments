#include <cstddef>

int a() {
    if constexpr (true) {
        return 0;
    } else {
        return std::byte {0x01};
    }
}

int main() {
    return 0; 
}
