#pragma once

#include "quant/data/time_series.hpp"

namespace quant::data {

void validate_market_data(
    const TimeSeries& series
);

} // namespace quant::data