#pragma once

#include "quant/backtest/backtest.hpp"
#include "quant/risk/performance.hpp"

#include <cstddef>
#include <vector>

namespace quant::research {

struct CostSensitivityPoint {
    double cost_multiplier{};

    quant::risk::PerformanceMetrics performance{};

    double total_transaction_cost{};
    double total_execution_cost{};
};

struct CostSensitivityResult {
    std::vector<CostSensitivityPoint> points;

    bool break_even_found{};
    double break_even_cost_multiplier{};
};

[[nodiscard]]
CostSensitivityResult analyze_cost_sensitivity(
    const std::vector<std::int64_t>& timestamps,
    const std::vector<double>& x_prices,
    const std::vector<double>& y_prices,
    const std::vector<int>& signals,
    const std::vector<double>& hedge_ratios,
    const quant::backtest::BacktestParameters& baseline_parameters,
    double periods_per_year,
    const std::vector<double>& cost_multipliers
);

} // namespace quant::research