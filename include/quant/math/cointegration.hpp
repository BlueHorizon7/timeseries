#pragma once

#include "quant/math/regression.hpp"
#include "quant/math/series.hpp"

namespace quant::math {

enum class CointegrationDecision {
    Cointegrated,
    NotCointegrated
};

struct CointegrationResult {
    LinearRegression regression;

    Series spread;

    double adf_statistic{};

    double critical_value_5pct{};

    CointegrationDecision decision{};
};

[[nodiscard]]
CointegrationResult
engle_granger(
    const Series& x,
    const Series& y
);

} // namespace quant::math