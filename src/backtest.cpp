#include "quant/backtest/backtest.hpp"
#include "quant/portfolio/pnl.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::backtest {

BacktestResult run_backtest(
    const std::vector<std::int64_t>& timestamps,
    const std::vector<double>& x_prices,
    const std::vector<double>& y_prices,
    const std::vector<int>& signals,
    const std::vector<double>& hedge_ratios,
    const BacktestParameters& parameters
) {
    const std::size_t n = timestamps.size();

    if (x_prices.size() != n ||
        y_prices.size() != n ||
        signals.size() != n ||
        hedge_ratios.size() != n) {
        throw std::invalid_argument(
            "Backtest inputs must have equal lengths"
        );
    }

    if (n < 2) {
        throw std::invalid_argument(
            "Backtest requires at least two observations"
        );
    }

    if (!std::isfinite(parameters.initial_capital) ||
        parameters.initial_capital <= 0.0) {
        throw std::invalid_argument(
            "Initial capital must be positive and finite"
        );
    }

    if (!std::isfinite(parameters.gross_notional) ||
        parameters.gross_notional < 0.0) {
        throw std::invalid_argument(
            "Gross notional must be non-negative and finite"
        );
    }

    for (std::size_t i = 1; i < n; ++i) {
        if (timestamps[i] <= timestamps[i - 1]) {
            throw std::invalid_argument(
                "Backtest timestamps must be strictly increasing"
            );
        }
    }

    BacktestResult result;

    result.bars.reserve(n - 1);

    quant::portfolio::PairPosition previous_position{};

    double equity =
        parameters.initial_capital;

    /*
        Signal at t determines the position held
        during t -> t+1.

        Therefore P&L for [t, t+1] uses the
        position generated at t.
    */

    for (std::size_t t = 0; t + 1 < n; ++t) {
        const auto current_position =
            quant::portfolio::construct_pair_position(
                signals[t],
                hedge_ratios[t],
                parameters.gross_notional
            );

        const auto pnl =
            quant::portfolio::calculate_pnl(
                current_position,
                x_prices[t],
                x_prices[t + 1],
                y_prices[t],
                y_prices[t + 1]
            );

        const double cost =
            quant::portfolio::transaction_cost(
                previous_position,
                current_position,
                parameters.transaction_costs
            );

        const double net_pnl =
            pnl.total_pnl - cost;

        equity += net_pnl;

        result.bars.push_back(
            BacktestBar{
                timestamps[t + 1],
                x_prices[t + 1],
                y_prices[t + 1],
                signals[t],
                current_position,
                pnl.total_pnl,
                cost,
                net_pnl,
                equity
            }
        );

        previous_position =
            current_position;
    }

    return result;
}

} // namespace quant::backtest