#include "quant/execution/execution_coordinator.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

int main()
{
    using namespace quant::execution;

    ExecutionEngine engine;

    PortfolioLedger ledger{};
    ledger.cash = 100000.0;

    SimulatedExecutionVenue venue({
        .buy_slippage_bps = 0.0,
        .sell_slippage_bps = 0.0,
        .commission_rate = 0.001,
        .partial_fill_fraction = 1.0,
        .latency = 0
    });

    ExecutionCoordinator coordinator(
        engine,
        ledger,
        venue
    );

    Order order{};
    order.symbol = "XYZ";
    order.side = OrderSide::Buy;
    order.type = OrderType::Market;
    order.quantity = 100.0;
    order.remaining_quantity = 100.0;

    const auto report =
        coordinator.submit_and_execute(
            order,
            {{"XYZ", 100.0}},
            1000
        );

    assert(report.order_id == 1);
    assert(report.fills.size() == 1);

    const Position& position_value =
        position(ledger, "XYZ");

    assert(position_value.quantity == 100.0);

    assert(
        std::abs(ledger.cash - 89990.0) < 1e-10
    );

    assert(
        std::abs(ledger.total_commission - 10.0) < 1e-10
    );

    std::cout << "execution_coordinator_test passed\n";
}