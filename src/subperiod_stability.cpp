#include "quant/research/subperiod_stability.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace quant::research {

SubperiodStabilityResult analyze_subperiod_stability(
    const quant::backtest::BacktestResult& backtest,
    double periods_per_year,
    std::size_t number_of_subperiods
) {
    if (backtest.bars.empty()) {
        throw std::invalid_argument(
            "Subperiod analysis requires a non-empty backtest"
        );
    }

    if (!std::isfinite(periods_per_year) ||
        periods_per_year <= 0.0) {
        throw std::invalid_argument(
            "Periods per year must be finite and positive"
        );
    }

    if (number_of_subperiods == 0 ||
        number_of_subperiods > backtest.bars.size()) {
        throw std::invalid_argument(
            "Invalid number of subperiods"
        );
    }

    const std::size_t n = backtest.bars.size();

    const std::size_t base_size =
        n / number_of_subperiods;

    const std::size_t remainder =
        n % number_of_subperiods;

    SubperiodStabilityResult result;
    result.periods.reserve(number_of_subperiods);

    std::size_t begin = 0;

    for (std::size_t period = 0;
         period < number_of_subperiods;
         ++period) {

        const std::size_t length =
            base_size +
            (period < remainder ? 1U : 0U);

        const std::size_t end =
            begin + length;

        quant::backtest::BacktestResult subperiod;

        subperiod.bars.reserve(length);
        subperiod.liquidation_cost = 0.0;

        /*
         * Recover the equity immediately before the first
         * bar of this subperiod.
         */
        double initial_equity = 0.0;

        if (begin == 0) {
            initial_equity =
                backtest.bars.front().equity -
                backtest.bars.front().net_pnl;
        } else {
            initial_equity =
                backtest.bars[begin - 1].equity;
        }

        if (!std::isfinite(initial_equity) ||
            initial_equity <= 0.0) {
            throw std::domain_error(
                "Subperiod initial equity must be finite and positive"
            );
        }

        double equity = initial_equity;

        for (std::size_t i = begin; i < end; ++i) {
            auto bar = backtest.bars[i];

            equity += bar.net_pnl;

            if (!std::isfinite(equity)) {
                throw std::domain_error(
                    "Subperiod equity became non-finite"
                );
            }

            bar.equity = equity;

            subperiod.bars.push_back(bar);
        }

        const auto performance =
            quant::risk::calculate_performance(
                subperiod,
                periods_per_year
            );

        result.periods.push_back(
            SubperiodPerformance{
                begin,
                end,
                performance
            }
        );

        begin = end;
    }

    if (result.periods.empty()) {
        throw std::runtime_error(
            "Subperiod analysis produced no periods"
        );
    }

    result.worst_total_return =
        std::numeric_limits<double>::infinity();

    result.best_total_return =
        -std::numeric_limits<double>::infinity();

    result.worst_sharpe =
        std::numeric_limits<double>::infinity();

    result.best_sharpe =
        -std::numeric_limits<double>::infinity();

    result.profitable_periods = 0;
    result.losing_periods = 0;

    for (const auto& period : result.periods) {
        result.worst_total_return =
            std::min(
                result.worst_total_return,
                period.performance.total_return
            );

        result.best_total_return =
            std::max(
                result.best_total_return,
                period.performance.total_return
            );

        result.worst_sharpe =
            std::min(
                result.worst_sharpe,
                period.performance.sharpe_ratio
            );

        result.best_sharpe =
            std::max(
                result.best_sharpe,
                period.performance.sharpe_ratio
            );

        if (period.performance.total_return > 0.0) {
            ++result.profitable_periods;
        } else if (period.performance.total_return < 0.0) {
            ++result.losing_periods;
        }
    }

    return result;
}

} // namespace quant::research