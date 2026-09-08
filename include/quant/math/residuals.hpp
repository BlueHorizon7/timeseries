#pragma once

#include "quant/math/regression.hpp"
#include "quant/math/series.hpp"

namespace quant::math {

[[nodiscard]]
Series residuals(
    const Series& x,
    const Series& y,
    const LinearRegression& model
);

} // namespace quant::math