#pragma once

namespace quant::portfolio {

struct PairPosition {
    double x_notional{};
    double y_notional{};
};

[[nodiscard]]
PairPosition construct_pair_position(
    int signal,
    double hedge_ratio,
    double gross_notional
);

} // namespace quant::portfolio