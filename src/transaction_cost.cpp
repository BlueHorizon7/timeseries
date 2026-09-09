#include "quant/portfolio/transaction_cost.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::portfolio {

double transaction_cost(
    const PairPosition& previous_position,
    const PairPosition& current_position,
    const TransactionCostParameters& parameters
) {
    if (!std::isfinite(parameters.x_cost_rate) ||
        !std::isfinite(parameters.y_cost_rate)) {
        throw std::domain_error(
            "Transaction-cost rates must be finite"
        );
    }

    if (parameters.x_cost_rate < 0.0 ||
        parameters.y_cost_rate < 0.0) {
        throw std::invalid_argument(
            "Transaction-cost rates must be non-negative"
        );
    }

    const double x_turnover =
        std::abs(
            current_position.x_notional -
            previous_position.x_notional
        );

    const double y_turnover =
        std::abs(
            current_position.y_notional -
            previous_position.y_notional
        );

    return
        x_turnover * parameters.x_cost_rate +
        y_turnover * parameters.y_cost_rate;
}

} // namespace quant::portfolio