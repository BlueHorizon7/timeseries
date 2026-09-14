#pragma once

#include "quant/portfolio/pair_position.hpp"

namespace quant::portfolio {

struct ExecutionCostParameters {
    double x_slippage_bps{};
    double y_slippage_bps{};

    double x_market_impact_bps{};
    double y_market_impact_bps{};

    double impact_reference_notional{1.0};
    double market_impact_exponent{1.0};
};

struct ExecutionCostResult {
    double x_trade_notional{};
    double y_trade_notional{};

    double x_slippage_cost{};
    double y_slippage_cost{};

    double x_market_impact_cost{};
    double y_market_impact_cost{};

    double total_slippage_cost{};
    double total_market_impact_cost{};
    double total_execution_cost{};
};

[[nodiscard]]
ExecutionCostResult calculate_execution_cost(
    const PairPosition& previous_position,
    const PairPosition& current_position,
    const ExecutionCostParameters& parameters
);

} // namespace quant::portfolio