#pragma once

#include "quant/math/series.hpp"

namespace quant::math {

[[nodiscard]]
double mean(const Series& series);

[[nodiscard]]
double variance(const Series& series);

[[nodiscard]]
double standard_deviation(const Series& series);

[[nodiscard]]
double covariance(
    const Series& x,
    const Series& y
);

[[nodiscard]]
double correlation(
    const Series& x,
    const Series& y
);

} // namespace quant::math