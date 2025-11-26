#include <print>
#include <memory>
#include <vector>

template<typename... Args>
class Observer {
public:
    virtual ~Observer() {}
    virtual void update(Args... args) = 0;
};

template<typename... Args>
class Observable {
private:

    std::vector<std::weak_ptr<Observer<Args...>>> observers;

public:

    virtual ~Observable() {}

    void addObserver(const std::weak_ptr<Observer<Args...>>& observer) {
        observers.push_back(observer);
    }

    void notifyListeners(Args... args) {
        for (auto& observer: observers) {
            if (auto observer_ptr = observer.lock()) {
                observer_ptr->update(args...);
            }
        }
    }
};

template<typename... Args>
class LogObserver: public Observer<Args...> {
public:
    void update(Args... args) override {
        (std::print("{} ", args), ...);
        std::println("");
    }
};


int main() {
    auto a = std::make_shared<LogObserver<int, int>>();
    auto b = std::make_shared<LogObserver<int, int>>();
    Observable<int, int> parent;
    parent.addObserver(a);
    parent.addObserver(b);

    parent.notifyListeners(5, 10);

    return 0;
}
