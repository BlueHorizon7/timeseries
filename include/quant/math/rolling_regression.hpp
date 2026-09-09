#pragma once

#include "quant/math/regression.hpp"
#include "quant/math/series.hpp"

#include <cstddef>
#include <vector>

namespace quant::math {

struct RollingRegressionResult {
    std::size_t timestamp_index{};
    LinearRegression model{};
};

[[nodiscard]]
std::vector<RollingRegressionResult>
rolling_ols(
    const Series& x,
    const Series& y,
    std::size_t window
);

} // namespace quant::math