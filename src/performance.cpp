#include "quant/risk/performance.hpp"

#include <cmath>
#include <stdexcept>
#include <vector>

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

    /*
     * The first bar contains the first period's P&L.
     * Therefore:
     *
     * initial equity =
     *     first equity - first period net P&L
     */
    const double initial_equity =
        bars.front().equity -
        bars.front().net_pnl;

    if (!std::isfinite(initial_equity) ||
        initial_equity <= 0.0) {
        throw std::domain_error(
            "Initial equity must be finite and positive"
        );
    }

    /*
     * Terminal liquidation is not a separate trading
     * period. It is a cost applied after the final
     * marked-to-market equity.
     */
    if (!std::isfinite(result.liquidation_cost) ||
        result.liquidation_cost < 0.0) {
        throw std::domain_error(
            "Liquidation cost must be finite and non-negative"
        );
    }

    const double final_equity =
        bars.back().equity -
        result.liquidation_cost;

    if (!std::isfinite(final_equity) ||
        final_equity <= 0.0) {
        throw std::domain_error(
            "Final equity after liquidation must be finite and positive"
        );
    }

    /*
     * Total P&L and total return.
     */
    const double total_pnl =
        final_equity - initial_equity;

    const double total_return =
        total_pnl / initial_equity;

    /*
     * Annualized geometric return.
     *
     * The liquidation cost is included in final equity,
     * so it is automatically included in annualized return.
     */
    const double number_of_periods =
        static_cast<double>(bars.size());

    const double annualized_return =
        std::pow(
            final_equity / initial_equity,
            periods_per_year / number_of_periods
        ) - 1.0;

    /*
     * Calculate period-by-period returns.
     *
     * Terminal liquidation is intentionally NOT included
     * as a period return because it is not another
     * trading interval.
     */
    std::vector<double> period_returns;
    period_returns.reserve(bars.size());

    for (std::size_t i = 0; i < bars.size(); ++i) {
        const double previous_equity =
            (i == 0)
                ? initial_equity
                : bars[i - 1].equity;

        if (!std::isfinite(previous_equity) ||
            previous_equity <= 0.0) {
            throw std::domain_error(
                "Cannot calculate returns from non-positive equity"
            );
        }

        const double period_return =
            bars[i].net_pnl / previous_equity;

        if (!std::isfinite(period_return)) {
            throw std::domain_error(
                "Period return must be finite"
            );
        }

        period_returns.push_back(period_return);
    }

    /*
     * Mean periodic return.
     */
    double return_sum = 0.0;

    for (const double period_return : period_returns) {
        return_sum += period_return;
    }

    const double mean_return =
        return_sum /
        static_cast<double>(period_returns.size());

    /*
     * Sample variance of periodic returns.
     */
    double variance = 0.0;

    if (period_returns.size() >= 2) {
        double squared_deviation_sum = 0.0;

        for (const double period_return : period_returns) {
            const double deviation =
                period_return - mean_return;

            squared_deviation_sum +=
                deviation * deviation;
        }

        variance =
            squared_deviation_sum /
            static_cast<double>(
                period_returns.size() - 1
            );
    }

    if (variance < 0.0) {
        /*
         * Variance cannot mathematically be negative.
         * Treat a tiny negative floating-point residue
         * as zero, but reject materially negative values.
         */
        if (variance > -1e-15) {
            variance = 0.0;
        } else {
            throw std::domain_error(
                "Return variance cannot be negative"
            );
        }
    }

    const double periodic_volatility =
        std::sqrt(variance);

    const double annualized_volatility =
        periodic_volatility *
        std::sqrt(periods_per_year);

    /*
     * Standard annualized Sharpe ratio with a zero
     * risk-free rate:
     *
     * Sharpe =
     *     mean periodic return
     *     / periodic volatility
     *     * sqrt(periods per year)
     */
    double sharpe_ratio = 0.0;

    if (periodic_volatility > 0.0) {
        sharpe_ratio =
            (mean_return / periodic_volatility) *
            std::sqrt(periods_per_year);
    }

    /*
     * Maximum drawdown.
     *
     * Normal trading bars are processed first.
     * Terminal liquidation is then treated as the
     * final realized equity point for drawdown purposes.
     *
     * It is NOT treated as a volatility/Sharpe period.
     */
    double peak_equity = initial_equity;
    double maximum_drawdown = 0.0;
    double maximum_drawdown_pct = 0.0;

    for (const auto& bar : bars) {
        if (!std::isfinite(bar.equity)) {
            throw std::domain_error(
                "Equity must be finite"
            );
        }

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
     * Include fully liquidated final equity in
     * maximum drawdown.
     */
    if (final_equity > peak_equity) {
        peak_equity = final_equity;
    }

    const double terminal_drawdown =
        peak_equity - final_equity;

    const double terminal_drawdown_pct =
        terminal_drawdown / peak_equity;

    if (terminal_drawdown > maximum_drawdown) {
        maximum_drawdown = terminal_drawdown;
    }

    if (terminal_drawdown_pct > maximum_drawdown_pct) {
        maximum_drawdown_pct = terminal_drawdown_pct;
    }

    /*
     * Count changes in the actual position.
     *
     * The initial transition:
     *
     *     flat -> first position
     *
     * counts as a position change.
     */
    std::size_t number_of_position_changes = 0;

    quant::portfolio::PairPosition previous_position{};

    for (const auto& bar : bars) {
        const auto& position = bar.position;

        if (position.x_notional !=
                previous_position.x_notional ||
            position.y_notional !=
                previous_position.y_notional) {
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