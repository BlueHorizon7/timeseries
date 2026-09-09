#pragma once

#include "quant/math/series.hpp"

namespace quant::strategy {

enum class Position {
    ShortSpread = -1,
    Flat = 0,
    LongSpread = 1
};

struct SignalParameters {
    double entry_zscore{2.0};
    double exit_zscore{0.5};
};

[[nodiscard]]
Position
classify_signal(
    double zscore,
    Position current_position,
    const SignalParameters& parameters
);

[[nodiscard]]
quant::math::Series
generate_signals(
    const quant::math::Series& zscores,
    const SignalParameters& parameters
);

} // namespace quant::strategy