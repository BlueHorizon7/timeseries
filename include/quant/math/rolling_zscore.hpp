#pragma once

#include "quant/math/series.hpp"

#include <cstddef>

namespace quant::math {

Series rolling_zscore(
    const Series& series,
    std::size_t window
);

} // namespace quant::math