#pragma once

#include "quant/math/series.hpp"

namespace quant::math {

struct AR1Result {
    double intercept{};
    double phi{};
};

[[nodiscard]]
AR1Result fit_ar1(const Series& series);

} // namespace quant::math