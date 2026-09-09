#pragma once

#include "quant/backtest/backtest.hpp"

#include <cstddef>

namespace quant::risk {

struct PerformanceMetrics {
    double initial_equity{};
    double final_equity{};
    double total_pnl{};
    double total_return{};
    double annualized_return{};
    double annualized_volatility{};
    double sharpe_ratio{};
    double maximum_drawdown{};
    double maximum_drawdown_pct{};

    std::size_t number_of_bars{};
    std::size_t number_of_position_changes{};
};

[[nodiscard]]
PerformanceMetrics
calculate_performance(
    const quant::backtest::BacktestResult& result,
    double periods_per_year
);

} // namespace quant::risk