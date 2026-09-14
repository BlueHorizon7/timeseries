#include "quant/math/cointegration.hpp"

#include "quant/math/adf.hpp"
#include "quant/math/adf_selection.hpp"
#include "quant/math/mackinnon.hpp"
#include "quant/math/regression.hpp"
#include "quant/math/residuals.hpp"

#include <stdexcept>

namespace quant::math {

namespace {

CointegrationResult run_engle_granger(
    const Series& x,
    const Series& y,
    std::size_t adf_lags,
    const LinearRegression& regression,
    const Series& spread,
    const ADFResult& adf
) {
    /*
        The residual ADF loses one observation because
        of first differencing.
    */
    const std::size_t effective_observations =
        x.size() - 1;

    const auto critical_values =
        mackinnon_cointegration_critical_values(
            effective_observations
        );

    const auto decision =
        adf.statistic <
                critical_values.five_percent
            ? CointegrationDecision::Cointegrated
            : CointegrationDecision::NotCointegrated;

    return CointegrationResult{
        regression,
        spread,
        adf.statistic,
        critical_values,
        decision,
        x.size(),
        adf_lags
    };
}

void validate_series(
    const Series& x,
    const Series& y
) {
    if (x.size() != y.size()) {
        throw std::invalid_argument(
            "Engle-Granger requires equal-length series"
        );
    }

    if (x.size() < 20) {
        throw std::invalid_argument(
            "Engle-Granger requires at least "
            "20 observations"
        );
    }

    for (std::size_t i = 0;
         i < x.size();
         ++i) {

        if (x[i].timestamp != y[i].timestamp) {
            throw std::invalid_argument(
                "Engle-Granger requires aligned timestamps"
            );
        }
    }
}

} // namespace

CointegrationResult engle_granger(
    const Series& x,
    const Series& y,
    std::size_t adf_lags
) {
    const CointegrationParameters parameters{
        false,
        adf_lags,
        0,
        InformationCriterion::AIC
    };

    return engle_granger(
        x,
        y,
        parameters
    );
}

CointegrationResult engle_granger(
    const Series& x,
    const Series& y,
    const CointegrationParameters&
        parameters
) {
    validate_series(x, y);

    const auto regression =
        ordinary_least_squares(
            x,
            y
        );

    const auto spread =
        residuals(
            x,
            y,
            regression
        );

    ADFResult adf;
    std::size_t selected_lags = 0;

    if (parameters.automatic_lag_selection) {
        if (parameters.max_adf_lags == 0) {
            /*
                max_adf_lags == 0 still means the candidate
                set contains only lag 0.
            */
            adf =
                augmented_dickey_fuller(
                    spread,
                    0,
                    DeterministicTerm::None
                );

            selected_lags = 0;
        } else {
            const auto selection =
                select_adf_lag(
                    spread,
                    parameters.max_adf_lags,
                    DeterministicTerm::None,
                    parameters.information_criterion
                );

            adf =
                selection.adf;

            selected_lags =
                selection.selected_lags;
        }
    } else {
        adf =
            augmented_dickey_fuller(
                spread,
                parameters.adf_lags,
                DeterministicTerm::None
            );

        selected_lags =
            parameters.adf_lags;
    }

    return run_engle_granger(
        x,
        y,
        selected_lags,
        regression,
        spread,
        adf
    );
}

} // namespace quant::math