#include "quant/math/cointegration.hpp"

#include "quant/math/adf.hpp"
#include "quant/math/residuals.hpp"

#include <stdexcept>

namespace quant::math {

CointegrationResult engle_granger(
    const Series& x,
    const Series& y
) {
    if (x.size() != y.size()) {
        throw std::invalid_argument(
            "Cointegration requires equal-length series"
        );
    }

    if (x.size() < 20) {
        throw std::invalid_argument(
            "Cointegration test requires more observations"
        );
    }

    for (std::size_t i = 0; i < x.size(); ++i) {
        if (x[i].timestamp != y[i].timestamp) {
            throw std::invalid_argument(
                "Cointegration requires aligned timestamps"
            );
        }
    }

    // Step 1:
    //
    // Y_t = alpha + beta X_t + epsilon_t

    const LinearRegression regression =
        ordinary_least_squares(x, y);

    // Step 2:
    //
    // epsilon_t = Y_t - alpha - beta X_t

    const Series spread =
        residuals(x, y, regression);

    // Step 3:
    //
    // Test the residual for a unit root.
    //
    // We use the no-lag Dickey-Fuller regression here.
    // The critical value below is specifically for the
    // residual-based Engle-Granger test.

    const ADFResult adf =
        augmented_dickey_fuller(
            spread,
            0,
            DeterministicTerm::None
        );

    // Approximate 5% asymptotic Engle-Granger
    // residual-based critical value for the
    // two-variable case.
    constexpr double critical_value_5pct = -3.34;

    const CointegrationDecision decision =
        adf.statistic < critical_value_5pct
            ? CointegrationDecision::Cointegrated
            : CointegrationDecision::NotCointegrated;

    return CointegrationResult{
        regression,
        spread,
        adf.statistic,
        critical_value_5pct,
        decision
    };
}

} // namespace quant::math