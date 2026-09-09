#pragma once

#include "quant/data/time_series.hpp"

#include <string>

namespace quant::data {

[[nodiscard]]
TimeSeries read_candles_csv(
    const std::string& filename
);

} // namespace quant::data