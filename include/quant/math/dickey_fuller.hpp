#pragma once

#include "quant/math/series.hpp"

namespace quant::math {

struct DickeyFullerResult {
    double intercept{};
    double gamma{};
    double standard_error{};
    double statistic{};
};

[[nodiscard]]
DickeyFullerResult dickey_fuller(
    const Series& series
);

} // namespace quant::math