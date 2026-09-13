#include "quant/math/cointegration.hpp"

#include "quant/math/adf.hpp"
#include "quant/math/mackinnon.hpp"
#include "quant/math/regression.hpp"
#include "quant/math/residuals.hpp"

#include <stdexcept>

namespace quant::math {

CointegrationResult engle_granger(
    const Series& x,
    const Series& y,
    std::size_t adf_lags
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

        if (x[i].timestamp !=
            y[i].timestamp) {

            throw std::invalid_argument(
                "Engle-Granger requires aligned timestamps"
            );
        }
    }

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

    /*
     * Engle-Granger second stage:
     *
     * Δe_t =
     *     γ e_{t-1}
     *   + Σ δ_i Δe_{t-i}
     *   + u_t
     *
     * No deterministic term is added here because
     * the first-stage cointegrating regression
     * already included the intercept.
     */
    const auto adf =
        augmented_dickey_fuller(
            spread,
            adf_lags,
            DeterministicTerm::None
        );

    /*
     * Match the finite-sample response-surface
     * convention used for the cointegration test.
     *
     * The residual ADF loses one observation because
     * of first differencing.
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

} // namespace quant::math