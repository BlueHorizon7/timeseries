#pragma once

#include "quant/data/time_series.hpp"
#include "quant/math/series.hpp"

namespace quant::math {

Series log_returns(const quant::data::TimeSeries& prices);

} // namespace quant::math