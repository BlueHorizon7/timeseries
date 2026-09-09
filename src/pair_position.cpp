#include "quant/portfolio/pair_position.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::portfolio {

PairPosition construct_pair_position(
    int signal,
    double hedge_ratio,
    double gross_notional
) {
    if (signal < -1 || signal > 1) {
        throw std::invalid_argument(
            "Signal must be -1, 0, or 1"
        );
    }

    if (!std::isfinite(hedge_ratio)) {
        throw std::domain_error(
            "Hedge ratio must be finite"
        );
    }

    if (!std::isfinite(gross_notional) ||
        gross_notional < 0.0) {
        throw std::invalid_argument(
            "Gross notional must be finite and non-negative"
        );
    }

    if (signal == 0 || gross_notional == 0.0) {
        return PairPosition{};
    }

    const double normalization =
        1.0 + std::abs(hedge_ratio);

    const double y_notional =
        static_cast<double>(signal) *
        gross_notional /
        normalization;

    const double x_notional =
        -static_cast<double>(signal) *
        hedge_ratio *
        gross_notional /
        normalization;

    return PairPosition{
        x_notional,
        y_notional
    };
}

} // namespace quant::portfolio