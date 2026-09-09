#pragma once

#include "quant/math/rolling_regression.hpp"
#include "quant/math/series.hpp"

namespace quant::math {

[[nodiscard]]
Series rolling_spread(
    const Series& x,
    const Series& y,
    std::size_t window
);

} // namespace quant::math