#include "quant/execution/fill.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::execution {

void validate_fill(
    const Fill& fill
) {
    if (fill.order_id == 0) {
        throw std::invalid_argument(
            "Fill order id must be non-zero"
        );
    }

    if (fill.symbol.empty()) {
        throw std::invalid_argument(
            "Fill symbol must not be empty"
        );
    }

    if (!std::isfinite(fill.quantity) ||
        fill.quantity <= 0.0) {

        throw std::invalid_argument(
            "Fill quantity must be finite and positive"
        );
    }

    if (!std::isfinite(fill.price) ||
        fill.price <= 0.0) {

        throw std::invalid_argument(
            "Fill price must be finite and positive"
        );
    }

    if (fill.timestamp < 0) {
        throw std::invalid_argument(
            "Fill timestamp must be non-negative"
        );
    }

    if (!std::isfinite(fill.commission) ||
        fill.commission < 0.0) {

        throw std::invalid_argument(
            "Fill commission must be finite and non-negative"
        );
    }
}

} // namespace quant::execution