#pragma once

#include "quant/execution/execution_engine.hpp"
#include "quant/execution/portfolio_ledger.hpp"
#include "quant/execution/simulated_venue.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace quant::execution {

struct ExecutionReport {
    OrderId order_id{};
    std::vector<Fill> fills;
};

class ExecutionCoordinator {
public:
    ExecutionCoordinator(
        ExecutionEngine& engine,
        PortfolioLedger& ledger,
        SimulatedExecutionVenue& venue
    );

    [[nodiscard]]
    ExecutionReport submit_and_execute(
        Order order,
        const std::unordered_map<std::string, double>& market_prices,
        std::int64_t timestamp
    );

private:
    ExecutionEngine& engine_;
    PortfolioLedger& ledger_;
    SimulatedExecutionVenue& venue_;
};

} // namespace quant::execution