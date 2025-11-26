#include <concepts>
#include <cassert>
#include <vector>
#include <functional>

template<typename CommandType, typename... Args>
concept ExecutableCommand = requires (CommandType command, Args... args) {
    { command.execute(args...) };
};

class AddCommand {
public:
    int execute(int a, int b) {
        return a + b;
    }
};

class MulCommand {
public:
    int execute(int a, int b) {
        return a * b;
    }
};

template<typename CommandType, typename... Args>
requires ExecutableCommand<CommandType, Args...>
auto invoke(CommandType&& command, Args... args) {
    return command.execute(args...);
}

int main() {
    assert(invoke(AddCommand{}, 1, 2) == 3);
    assert(invoke(MulCommand{}, 2, 4) == 8);

    return 0;
}
