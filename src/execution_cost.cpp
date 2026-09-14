#include "quant/portfolio/execution_cost.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::portfolio {

ExecutionCostResult calculate_execution_cost(
    const PairPosition& previous_position,
    const PairPosition& current_position,
    const ExecutionCostParameters& parameters
) {
    if (!std::isfinite(parameters.x_slippage_bps) ||
        parameters.x_slippage_bps < 0.0) {
        throw std::invalid_argument(
            "X slippage must be finite and non-negative"
        );
    }

    if (!std::isfinite(parameters.y_slippage_bps) ||
        parameters.y_slippage_bps < 0.0) {
        throw std::invalid_argument(
            "Y slippage must be finite and non-negative"
        );
    }

    if (!std::isfinite(parameters.x_market_impact_bps) ||
        parameters.x_market_impact_bps < 0.0) {
        throw std::invalid_argument(
            "X market impact must be finite and non-negative"
        );
    }

    if (!std::isfinite(parameters.y_market_impact_bps) ||
        parameters.y_market_impact_bps < 0.0) {
        throw std::invalid_argument(
            "Y market impact must be finite and non-negative"
        );
    }

    if (!std::isfinite(parameters.impact_reference_notional) ||
        parameters.impact_reference_notional <= 0.0) {
        throw std::invalid_argument(
            "Impact reference notional must be finite and positive"
        );
    }

    if (!std::isfinite(parameters.market_impact_exponent) ||
        parameters.market_impact_exponent <= 0.0) {
        throw std::invalid_argument(
            "Market impact exponent must be finite and positive"
        );
    }

    const double x_trade_notional =
        std::abs(
            current_position.x_notional -
            previous_position.x_notional
        );

    const double y_trade_notional =
        std::abs(
            current_position.y_notional -
            previous_position.y_notional
        );

    const double x_slippage_cost =
        x_trade_notional *
        parameters.x_slippage_bps /
        10000.0;

    const double y_slippage_cost =
        y_trade_notional *
        parameters.y_slippage_bps /
        10000.0;

    const double x_trade_fraction =
        x_trade_notional /
        parameters.impact_reference_notional;

    const double y_trade_fraction =
        y_trade_notional /
        parameters.impact_reference_notional;

    const double x_market_impact_cost =
        x_trade_notional *
        parameters.x_market_impact_bps /
        10000.0 *
        std::pow(
            x_trade_fraction,
            parameters.market_impact_exponent
        );

    const double y_market_impact_cost =
        y_trade_notional *
        parameters.y_market_impact_bps /
        10000.0 *
        std::pow(
            y_trade_fraction,
            parameters.market_impact_exponent
        );

    return ExecutionCostResult{
        x_trade_notional,
        y_trade_notional,

        x_slippage_cost,
        y_slippage_cost,

        x_market_impact_cost,
        y_market_impact_cost,

        x_slippage_cost + y_slippage_cost,

        x_market_impact_cost + y_market_impact_cost,

        x_slippage_cost +
        y_slippage_cost +
        x_market_impact_cost +
        y_market_impact_cost
    };
}

} // namespace quant::portfolio