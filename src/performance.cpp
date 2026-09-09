#include "quant/risk/performance.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::risk {

PerformanceMetrics calculate_performance(
    const quant::backtest::BacktestResult& result,
    double periods_per_year
) {
    if (result.bars.empty()) {
        throw std::invalid_argument(
            "Performance analysis requires at least one bar"
        );
    }

    if (!std::isfinite(periods_per_year) ||
        periods_per_year <= 0.0) {
        throw std::invalid_argument(
            "Periods per year must be finite and positive"
        );
    }

    const auto& bars = result.bars;

    const double initial_equity =
        bars.front().equity - bars.front().net_pnl;

    const double final_equity =
        bars.back().equity;

    if (!std::isfinite(initial_equity) ||
        initial_equity <= 0.0) {
        throw std::domain_error(
            "Initial equity must be finite and positive"
        );
    }

    if (!std::isfinite(final_equity)) {
        throw std::domain_error(
            "Final equity must be finite"
        );
    }

    const double total_pnl =
        final_equity - initial_equity;

    const double total_return =
        total_pnl / initial_equity;

    const double number_of_periods =
        static_cast<double>(bars.size());

    const double annualized_return =
        std::pow(
            final_equity / initial_equity,
            periods_per_year / number_of_periods
        ) - 1.0;

    /*
     * Calculate volatility from period-by-period
     * equity returns.
     */
    double return_sum = 0.0;
    double return_squared_sum = 0.0;

    for (std::size_t i = 0; i < bars.size(); ++i) {
        const double previous_equity =
            (i == 0)
                ? initial_equity
                : bars[i - 1].equity;

        if (previous_equity <= 0.0) {
            throw std::domain_error(
                "Cannot calculate returns from non-positive equity"
            );
        }

        const double period_return =
            bars[i].net_pnl / previous_equity;

        return_sum += period_return;
        return_squared_sum +=
            period_return * period_return;
    }

    const double mean_return =
        return_sum / number_of_periods;

    double variance = 0.0;

    if (bars.size() >= 2) {
        variance =
            (return_squared_sum -
             number_of_periods *
             mean_return *
             mean_return) /
            (number_of_periods - 1.0);

        if (variance < 0.0 &&
            variance > -1e-15) {
            variance = 0.0;
        }
    }

    const double periodic_volatility =
        std::sqrt(variance);

    const double annualized_volatility =
        periodic_volatility *
        std::sqrt(periods_per_year);

    double sharpe_ratio = 0.0;

    if (annualized_volatility > 0.0) {
        sharpe_ratio =
            annualized_return /
            annualized_volatility;
    }

    /*
     * Maximum drawdown.
     */
    double peak_equity = initial_equity;
    double maximum_drawdown = 0.0;
    double maximum_drawdown_pct = 0.0;

    for (const auto& bar : bars) {
        if (bar.equity > peak_equity) {
            peak_equity = bar.equity;
        }

        const double drawdown =
            peak_equity - bar.equity;

        const double drawdown_pct =
            drawdown / peak_equity;

        if (drawdown > maximum_drawdown) {
            maximum_drawdown = drawdown;
        }

        if (drawdown_pct > maximum_drawdown_pct) {
            maximum_drawdown_pct = drawdown_pct;
        }
    }

    /*
     * Count changes in the actual position.
     */
    std::size_t number_of_position_changes = 0;

    quant::portfolio::PairPosition previous_position{};

    for (const auto& bar : bars) {
        const auto& position = bar.position;

        if (position.x_notional != previous_position.x_notional ||
            position.y_notional != previous_position.y_notional) {
            ++number_of_position_changes;
        }

        previous_position = position;
    }

    return PerformanceMetrics{
        initial_equity,
        final_equity,
        total_pnl,
        total_return,
        annualized_return,
        annualized_volatility,
        sharpe_ratio,
        maximum_drawdown,
        maximum_drawdown_pct,
        bars.size(),
        number_of_position_changes
    };
}

} // namespace quant::risk