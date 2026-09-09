#pragma once

#include "quant/portfolio/pair_position.hpp"
#include "quant/portfolio/transaction_cost.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace quant::backtest {

struct BacktestBar {
    std::int64_t timestamp{};
    double x_price{};
    double y_price{};
    int signal{};
    quant::portfolio::PairPosition position{};
    double gross_pnl{};
    double transaction_cost{};
    double net_pnl{};
    double equity{};
};

struct BacktestParameters {
    double initial_capital{};
    double gross_notional{};
    quant::portfolio::TransactionCostParameters transaction_costs{};
};

struct BacktestResult {
    std::vector<BacktestBar> bars;
};

[[nodiscard]]
BacktestResult run_backtest(
    const std::vector<std::int64_t>& timestamps,
    const std::vector<double>& x_prices,
    const std::vector<double>& y_prices,
    const std::vector<int>& signals,
    const std::vector<double>& hedge_ratios,
    const BacktestParameters& parameters
);

} // namespace quant::backtest