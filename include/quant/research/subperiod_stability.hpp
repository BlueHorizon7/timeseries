#pragma once

#include "quant/backtest/backtest.hpp"
#include "quant/risk/performance.hpp"

#include <cstddef>
#include <vector>

namespace quant::research {

struct SubperiodPerformance {
    std::size_t begin_bar{};
    std::size_t end_bar{};

    quant::risk::PerformanceMetrics performance{};
};

struct SubperiodStabilityResult {
    std::vector<SubperiodPerformance> periods;

    double worst_total_return{};
    double best_total_return{};

    double worst_sharpe{};
    double best_sharpe{};

    std::size_t profitable_periods{};
    std::size_t losing_periods{};
};

[[nodiscard]]
SubperiodStabilityResult analyze_subperiod_stability(
    const quant::backtest::BacktestResult& backtest,
    double periods_per_year,
    std::size_t number_of_subperiods
);

} // namespace quant::research