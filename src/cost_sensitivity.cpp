#include "quant/research/cost_sensitivity.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace quant::research {

namespace {

void validate_multiplier(double multiplier) {
    if (!std::isfinite(multiplier) ||
        multiplier < 0.0) {
        throw std::invalid_argument(
            "Cost multiplier must be finite and non-negative"
        );
    }
}

double interpolate_break_even(
    double left_multiplier,
    double left_pnl,
    double right_multiplier,
    double right_pnl
) {
    if (right_pnl == left_pnl) {
        return left_multiplier;
    }

    const double fraction =
        -left_pnl / (right_pnl - left_pnl);

    return
        left_multiplier +
        fraction *
        (right_multiplier - left_multiplier);
}

} // namespace

CostSensitivityResult analyze_cost_sensitivity(
    const std::vector<std::int64_t>& timestamps,
    const std::vector<double>& x_prices,
    const std::vector<double>& y_prices,
    const std::vector<int>& signals,
    const std::vector<double>& hedge_ratios,
    const quant::backtest::BacktestParameters& baseline_parameters,
    double periods_per_year,
    const std::vector<double>& cost_multipliers
) {
    if (cost_multipliers.empty()) {
        throw std::invalid_argument(
            "Cost sensitivity requires at least one multiplier"
        );
    }

    if (!std::isfinite(periods_per_year) ||
        periods_per_year <= 0.0) {
        throw std::invalid_argument(
            "Periods per year must be finite and positive"
        );
    }

    for (const double multiplier : cost_multipliers) {
        validate_multiplier(multiplier);
    }

    CostSensitivityResult result;

    result.points.reserve(cost_multipliers.size());

    for (const double multiplier : cost_multipliers) {
        auto parameters = baseline_parameters;

        parameters.transaction_costs.x_cost_rate *= multiplier;
        parameters.transaction_costs.y_cost_rate *= multiplier;

        parameters.execution_costs.x_slippage_bps *= multiplier;
        parameters.execution_costs.y_slippage_bps *= multiplier;

        parameters.execution_costs.x_market_impact_bps *= multiplier;
        parameters.execution_costs.y_market_impact_bps *= multiplier;

        const auto backtest =
            quant::backtest::run_backtest(
                timestamps,
                x_prices,
                y_prices,
                signals,
                hedge_ratios,
                parameters
            );

        const auto performance =
            quant::risk::calculate_performance(
                backtest,
                periods_per_year
            );

        double transaction_cost = 0.0;
        double execution_cost = 0.0;

        for (const auto& bar : backtest.bars) {
            transaction_cost += bar.transaction_cost;
            execution_cost += bar.execution_cost;
        }

        transaction_cost +=
            parameters.transaction_costs.x_cost_rate >= 0.0
                ? 0.0
                : 0.0;

        result.points.push_back(
            CostSensitivityPoint{
                multiplier,
                performance,
                transaction_cost,
                execution_cost
            }
        );
    }

    std::vector<std::size_t> order(
        result.points.size()
    );

    for (std::size_t i = 0; i < order.size(); ++i) {
        order[i] = i;
    }

    std::sort(
        order.begin(),
        order.end(),
        [&](std::size_t lhs, std::size_t rhs) {
            return
                result.points[lhs].cost_multiplier <
                result.points[rhs].cost_multiplier;
        }
    );

    for (std::size_t i = 0; i < order.size(); ++i) {
        const auto& point =
            result.points[order[i]];

        if (point.performance.total_pnl == 0.0) {
            result.break_even_found = true;
            result.break_even_cost_multiplier =
                point.cost_multiplier;
            break;
        }

        if (i + 1 >= order.size()) {
            continue;
        }

        const auto& next =
            result.points[order[i + 1]];

        const double pnl_a =
            point.performance.total_pnl;

        const double pnl_b =
            next.performance.total_pnl;

        if ((pnl_a > 0.0 && pnl_b < 0.0) ||
            (pnl_a < 0.0 && pnl_b > 0.0)) {

            result.break_even_found = true;

            result.break_even_cost_multiplier =
                interpolate_break_even(
                    point.cost_multiplier,
                    pnl_a,
                    next.cost_multiplier,
                    pnl_b
                );

            break;
        }
    }

    return result;
}

} // namespace quant::research