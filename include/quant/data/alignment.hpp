#pragma once

#include "quant/data/time_series.hpp"
#include "quant/data/aligned_series.hpp"

namespace quant::data {

[[nodiscard]]
AlignedSeries
inner_join(
    const TimeSeries& x,
    const TimeSeries& y
);

} // namespace quant::data