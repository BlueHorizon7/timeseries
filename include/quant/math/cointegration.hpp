#pragma once

#include "quant/math/adf_selection.hpp"
#include "quant/math/mackinnon.hpp"
#include "quant/math/regression.hpp"
#include "quant/math/series.hpp"

#include <cstddef>

namespace quant::math {

enum class CointegrationDecision {
    Cointegrated,
    NotCointegrated
};

struct CointegrationParameters {
    /*
        If automatic_lag_selection is false, adf_lags is used
        explicitly.

        If automatic_lag_selection is true, lags are selected
        from [0, max_adf_lags] using the requested information
        criterion.
    */
    bool automatic_lag_selection{false};

    std::size_t adf_lags{};

    std::size_t max_adf_lags{};

    InformationCriterion
        information_criterion{
            InformationCriterion::AIC
        };
};

struct CointegrationResult {
    LinearRegression regression;
    Series spread;

    double adf_statistic{};

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
    std::size_t adf_lags = 0
);

[[nodiscard]]
CointegrationResult engle_granger(
    const Series& x,
    const Series& y,
    const CointegrationParameters&
        parameters
);

} // namespace quant::math