#include "quant/portfolio/pnl.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::portfolio {

PnLResult calculate_pnl(
    const PairPosition& position,
    double previous_x_price,
    double current_x_price,
    double previous_y_price,
    double current_y_price
) {
    if (!std::isfinite(previous_x_price) ||
        !std::isfinite(current_x_price) ||
        !std::isfinite(previous_y_price) ||
        !std::isfinite(current_y_price)) {
        throw std::domain_error(
            "Prices must be finite"
        );
    }

    if (previous_x_price <= 0.0 ||
        current_x_price <= 0.0 ||
        previous_y_price <= 0.0 ||
        current_y_price <= 0.0) {
        throw std::domain_error(
            "Prices must be strictly positive"
        );
    }

    const double x_return =
        (current_x_price - previous_x_price) /
        previous_x_price;

    const double y_return =
        (current_y_price - previous_y_price) /
        previous_y_price;

    const double x_pnl =
        position.x_notional * x_return;

    const double y_pnl =
        position.y_notional * y_return;

    return PnLResult{
        x_pnl,
        y_pnl,
        x_pnl + y_pnl
    };
}

} // namespace quant::portfolio