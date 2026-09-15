#include "quant/execution/execution_coordinator.hpp"
#include "quant/execution/reconciliation.hpp"
#include "quant/execution/simulated_venue.hpp"

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

    Order order{};
    order.symbol = "XYZ";
    order.side = OrderSide::Buy;
    order.type = OrderType::Market;
    order.quantity = 25.0;
    order.remaining_quantity = 25.0;

   static_cast<void>(
    coordinator.submit_and_execute(
        order,
        {{"XYZ", 100.0}},
        1
    )
);

    const auto report =
        reconcile(engine, ledger);

    assert(report.consistent);

    std::cout << "reconciliation_test passed\n";
}