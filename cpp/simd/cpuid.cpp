#include <iostream>
#include <experimental/simd>

int main() {
    using simd = std::experimental::native_simd<int>;
    simd a(1);
    simd b(2);
    simd c = a + b;

    std::cout << c[0] << '\n';

    return 0;
}

