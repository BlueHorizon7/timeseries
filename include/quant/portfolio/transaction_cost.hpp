#pragma once

#include "quant/portfolio/pair_position.hpp"

namespace quant::portfolio {

struct TransactionCostParameters {
    double x_cost_rate{};
    double y_cost_rate{};
};

[[nodiscard]]
double transaction_cost(
    const PairPosition& previous_position,
    const PairPosition& current_position,
    const TransactionCostParameters& parameters
);

} // namespace quant::portfolio