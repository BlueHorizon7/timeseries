#pragma once

#include "quant/math/mackinnon.hpp"
#include "quant/math/regression.hpp"
#include "quant/math/series.hpp"

#include <cstddef>

namespace quant::math {

enum class CointegrationDecision {
    Cointegrated,
    NotCointegrated
};

struct CointegrationResult {
    LinearRegression regression;

    Series spread;

    double adf_statistic{};

    MacKinnonCriticalValues critical_values{};

    CointegrationDecision decision{};

    std::size_t observations{};

    std::size_t adf_lags{};
};

[[nodiscard]]
CointegrationResult engle_granger(
    const Series& x,
    const Series& y,
    std::size_t adf_lags = 0
);

} // namespace quant::math