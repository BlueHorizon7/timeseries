#include "quant/execution/execution_coordinator.hpp"

#include <stdexcept>

namespace quant::execution {

ExecutionCoordinator::ExecutionCoordinator(
    ExecutionEngine& engine,
    PortfolioLedger& ledger,
    SimulatedExecutionVenue& venue)
    : engine_(engine),
      ledger_(ledger),
      venue_(venue)
{
}

ExecutionReport ExecutionCoordinator::submit_and_execute(
    Order order,
    const std::unordered_map<std::string, double>& market_prices,
    std::int64_t timestamp)
{
    const auto market_it = market_prices.find(order.symbol);

    if (market_it == market_prices.end()) {
        throw std::invalid_argument(
            "missing market price for order symbol"
        );
    }

    const OrderId order_id =
        engine_.submit(std::move(order), timestamp);

    const Order& submitted = engine_.order(order_id);

    const std::vector<Fill> fills =
        venue_.execute(
            submitted,
            market_it->second,
            timestamp
        );

    for (const Fill& fill : fills) {
        engine_.process_fill(fill);
        apply_fill(ledger_, fill);
    }

    return ExecutionReport{
        order_id,
        fills
    };
}

} // namespace quant::execution