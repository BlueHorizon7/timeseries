#pragma once

#include "quant/math/adf_selection.hpp"
#include "quant/math/mackinnon.hpp"
#include "quant/math/regression.hpp"
#include "quant/math/series.hpp"

#include <cstddef>

namespace quant::math {

struct CointegrationParameters {
    bool automatic_lag_selection{};

    std::size_t adf_lags{};

    std::size_t max_adf_lags{};

    InformationCriterion
        information_criterion{
            InformationCriterion::AIC
        };
};

enum class CointegrationDecision {
    Cointegrated,
    NotCointegrated
};

struct CointegrationResult {
    LinearRegression regression;

    Series spread;

    double adf_statistic{};

    double p_value{};

    MacKinnonCriticalValues
        critical_values{};

    CointegrationDecision
        decision{};

    std::size_t observations{};

    std::size_t adf_lags{};
};

[[nodiscard]]
CointegrationResult engle_granger(
    const Series& x,
    const Series& y,
    const CointegrationParameters&
        parameters
);

[[nodiscard]]
CointegrationResult engle_granger(
    const Series& x,
    const Series& y,
    std::size_t adf_lags = 0
);

} // namespace quant::math