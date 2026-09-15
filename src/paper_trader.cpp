#include "quant/execution/paper_trader.hpp"

#include <stdexcept>

namespace quant::execution {

PaperTrader::PaperTrader(
    PortfolioLedger& ledger,
    ExecutionCoordinator& coordinator)
    : ledger_(ledger),
      coordinator_(coordinator)
{
}

PaperTradingResult PaperTrader::run(
    const std::vector<std::int64_t>& timestamps,
    const std::vector<
        std::unordered_map<std::string, double>>& market_prices,
    const std::vector<std::vector<TargetPosition>>& targets)
{
    if (timestamps.size() != market_prices.size() ||
        timestamps.size() != targets.size()) {
        throw std::invalid_argument(
            "paper trading inputs must have equal lengths"
        );
    }

    PaperTradingResult result{};
    result.bars.reserve(timestamps.size());

    std::vector<TargetPosition> current_positions;

    for (std::size_t i = 0; i < timestamps.size(); ++i) {
        const auto timestamp = timestamps[i];

        std::vector<TargetPosition> current_targets;

        for (const Position& position_value :
             positions(ledger_)) {

            current_targets.push_back({
                position_value.symbol,
                position_value.quantity
            });
        }

        const std::vector<Order> orders =
            generate_orders(
                current_targets,
                targets[i],
                timestamp
            );

        std::vector<ExecutionReport> reports;
        reports.reserve(orders.size());

        for (const Order& order : orders) {
            reports.push_back(
                coordinator_.submit_and_execute(
                    order,
                    market_prices[i],
                    timestamp
                )
            );
        }

        mark_to_market(
            ledger_,
            market_prices[i],
            timestamp
        );

        PaperTradingBar bar{};
        bar.timestamp = timestamp;
        bar.market_prices = market_prices[i];
        bar.target_positions = targets[i];
        bar.execution_reports = std::move(reports);
        bar.equity = equity(
            ledger_,
            market_prices[i]
        );

        result.bars.push_back(std::move(bar));
    }

    return result;
}

} // namespace quant::execution