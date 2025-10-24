#include <iostream>
#include <stdalign.h>
#include <string>

struct Unordered { 
    char a;
    int b;
    char c;
    short d;
};

struct Ordered { 
    char a;
    char c;
    short d;
    int b;
};

struct UnorderedStr { 
    char a;
    const char c[10]; // so: 10 // ao 1
    std::string b;    // so: 24 // ao: 8
};

struct OrderedStr { 
    char a;
    std::string b;    // so: 24 // ao: 8
    const char c[10]; // so: 10 // ao 1
};

int main() {
    std::cout << "all units are represented in bytes\n";
    std::cout << "I am naively ordering by size of members\n";
    std::cout << "Ordered      = " << sizeof(Ordered) << '\n';
    std::cout << "Unordered    = " << sizeof(Unordered) << '\n';
    std::cout << "UnorderedStr = " << sizeof(UnorderedStr) << '\n';
    std::cout << "OrderedStr   = " << sizeof(OrderedStr) << '\n';

    std::cout << "=== ALIGNMENT\n";
    std::cout << "char         = " << alignof(char) << '\n';
    std::cout << "int          = " << alignof(int) << '\n';
    std::cout << "char[10]     = " << alignof(char[10]) << '\n';
    std::cout << "std::string  = " << alignof(std::string) << '\n'; 
    std::cout << "Ordered      = " << alignof(Ordered) << '\n';
    std::cout << "Unordered    = " << alignof(Unordered) << '\n';
    std::cout << "OrderedStr   = " << alignof(OrderedStr) << '\n';
    std::cout << "UnorderedStr = " << alignof(UnorderedStr) << '\n';

    return 0;
}
