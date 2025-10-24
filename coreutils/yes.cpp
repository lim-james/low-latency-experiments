#include <print>

int main(int argsc, const char* argsv[]) {
    const char* msg = argsc <= 1 ? "y" : argsv[1];
    while (true) std::println("{}", msg); 
    return 0; 
}
