// market-data fetcher that talks to multiple inconsistent vendor APIs
// lightweight backtesting engine
// risk checker that enforces position limits before any simulated trade is accepted

#include <print>
#include <type_traits>

struct Tick {
    int symbol;
    int spot;
};

class VendorA {
public:
    Tick fetch_tick() { return {1,1}; }
};

class VendorB {
public:
    Tick fetch_tick() { return {1,2}; }
};

class VendorC {
public:
    Tick fetch_tick() { return {1,3}; }
};

template<typename Vendor>
concept MarketDataFetcher = requires (Vendor vendor) {
    { vendor.fetch_tick() } -> std::same_as<Tick>; 
};

class BacktestingEngine {
public:
    void execute_trade(const Tick& tick) {
        std::println("executing trade for {}", tick.symbol);
    }
};

class RiskChecker {
public:
    bool assess_risk_of_tick(const Tick& tick) {
        return tick.spot > 2;
    }
};

template<MarketDataFetcher Vendor>
class BacktestAnalystFacade {
private:

    Vendor vendor;
    BacktestingEngine backtestingEngine;
    RiskChecker riskChecker;

public:

    void backtest_vendor() {
        Tick tick = vendor.fetch_tick();
        if (riskChecker.assess_risk_of_tick(tick)) {
            backtestingEngine.execute_trade(tick);
        }
    }
};

int main() {
    BacktestAnalystFacade<VendorA> backtest;
    backtest.backtest_vendor();
    return 0;
}
