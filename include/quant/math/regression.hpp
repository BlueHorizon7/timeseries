#pragma once

#include "quant/math/series.hpp"

namespace quant::math {

struct LinearRegression {
    double intercept{};
    double slope{};
};

[[nodiscard]]
LinearRegression
ordinary_least_squares(
    const Series& x,
    const Series& y
);

} // namespace quant::math