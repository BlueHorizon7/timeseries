#include "quant/math/adf_selection.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <stdexcept>

int main() {
    using quant::math::DeterministicTerm;
    using quant::math::InformationCriterion;
    using quant::math::Observation;
    using quant::math::Series;

    /*
        Deterministic stationary process with serially
        structured disturbances.

        The exact optimal lag is not hard-coded here.
        The test verifies properties of the selector
        and consistency with the selected ADF model.
    */
    Series series;

    double value = 0.0;

    for (std::size_t i = 0; i < 200; ++i) {
        const double innovation =
            0.5 *
            std::sin(
                static_cast<double>(i) * 0.31
            );

        value =
            0.65 * value +
            innovation;

        series.add(
            Observation{
                static_cast<std::int64_t>(i),
                value
            }
        );
    }

    const auto aic_result =
        quant::math::select_adf_lag(
            series,
            8,
            DeterministicTerm::Intercept,
            InformationCriterion::AIC
        );

    const auto bic_result =
        quant::math::select_adf_lag(
            series,
            8,
            DeterministicTerm::Intercept,
            InformationCriterion::BIC
        );

    /*
        Selected lag must lie inside the requested
        candidate range.
    */
    assert(aic_result.selected_lags <= 8);
    assert(bic_result.selected_lags <= 8);

    /*
        The reported ADF result must correspond to
        the selected lag.
    */
    assert(
        aic_result.adf.lags ==
        aic_result.selected_lags
    );

    assert(
        bic_result.adf.lags ==
        bic_result.selected_lags
    );

    /*
        Deterministic specification must be preserved.
    */
    assert(
        aic_result.adf.deterministic ==
        DeterministicTerm::Intercept
    );

    assert(
        bic_result.adf.deterministic ==
        DeterministicTerm::Intercept
    );

    /*
        Criterion values and ADF statistics must be finite.
    */
    assert(
        std::isfinite(
            aic_result.criterion_value
        )
    );

    assert(
        std::isfinite(
            bic_result.criterion_value
        )
    );

    assert(
        std::isfinite(
            aic_result.adf.statistic
        )
    );

    assert(
        std::isfinite(
            bic_result.adf.statistic
        )
    );

    /*
        The selected ADF standard errors must be valid.
    */
    assert(
        aic_result.adf.standard_error > 0.0
    );

    assert(
        bic_result.adf.standard_error > 0.0
    );

    /*
        Invalid candidate range.
    */
    bool threw = false;

    try {
        (void)quant::math::select_adf_lag(
            series,
            series.size(),
            DeterministicTerm::Intercept,
            InformationCriterion::AIC
        );
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    return 0;
}