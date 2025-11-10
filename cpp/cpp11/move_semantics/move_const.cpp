#include <move>

int main() {
    const int x = 5;
    int y = std::move(x); // fails silently (makes copy)
    return 0;
}
