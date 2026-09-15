#pragma once

#include "quant/execution/execution_coordinator.hpp"
#include "quant/execution/order_generation.hpp"
#include "quant/execution/portfolio_ledger.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace quant::execution {

struct PaperTradingBar {
    std::int64_t timestamp{};

    std::unordered_map<std::string, double> market_prices;

    std::vector<TargetPosition> target_positions;

    std::vector<ExecutionReport> execution_reports;

    double equity{};
};

struct PaperTradingResult {
    std::vector<PaperTradingBar> bars;
};

class PaperTrader {
public:
    PaperTrader(
        PortfolioLedger& ledger,
        ExecutionCoordinator& coordinator
    );

    [[nodiscard]]
    PaperTradingResult run(
        const std::vector<std::int64_t>& timestamps,
        const std::vector<std::unordered_map<std::string, double>>&
            market_prices,
        const std::vector<std::vector<TargetPosition>>&
            targets
    );

private:
    PortfolioLedger& ledger_;
    ExecutionCoordinator& coordinator_;
};

} // namespace quant::execution