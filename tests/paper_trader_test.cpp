#include "quant/execution/paper_trader.hpp"

#include <cassert>
#include <iostream>

int main()
{
    using namespace quant::execution;

    ExecutionEngine engine;

    PortfolioLedger ledger{};
    ledger.cash = 100000.0;

    SimulatedExecutionVenue venue({
        .commission_rate = 0.0
    });

    ExecutionCoordinator coordinator(
        engine,
        ledger,
        venue
    );

    PaperTrader trader(
        ledger,
        coordinator
    );

    const std::vector<std::int64_t> timestamps{
        1, 2, 3
    };

    const std::vector<
        std::unordered_map<std::string, double>> prices{
        {{"XYZ", 100.0}},
        {{"XYZ", 105.0}},
        {{"XYZ", 110.0}}
    };

    const std::vector<std::vector<TargetPosition>> targets{
        {{"XYZ", 100.0}},
        {{"XYZ", 100.0}},
        {{"XYZ", 0.0}}
    };

    const auto result =
        trader.run(
            timestamps,
            prices,
            targets
        );

    assert(result.bars.size() == 3);

    const Position& final_position =
        position(ledger, "XYZ");

    assert(final_position.quantity == 0.0);

    assert(
        ledger.realized_pnl > 900.0
    );

    std::cout << "paper_trader_test passed\n";
}