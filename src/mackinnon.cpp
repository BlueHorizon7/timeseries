#include "quant/math/mackinnon.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::math {

namespace {

double evaluate_response_surface(
    double c0,
    double c1,
    double c2,
    double c3,
    double observations
) {
    const double inverse_n =
        1.0 / observations;

    const double inverse_n_squared =
        inverse_n * inverse_n;

    const double inverse_n_cubed =
        inverse_n_squared * inverse_n;

    return
        c0 +
        c1 * inverse_n +
        c2 * inverse_n_squared +
        c3 * inverse_n_cubed;
}

} // namespace

MacKinnonCriticalValues
mackinnon_cointegration_critical_values(
    std::size_t observations
) {
    if (observations < 2) {
        throw std::invalid_argument(
            "MacKinnon critical values require "
            "at least two observations"
        );
    }

    const double n =
        static_cast<double>(observations);

    // MacKinnon 2010 response-surface coefficients.
    //
    // N = 2:
    // one dependent variable + one regressor.
    //
    // Deterministic specification:
    // constant, no time trend.
    //
    // Polynomial:
    //
    // critical =
    //     c0
    //   + c1 / n
    //   + c2 / n^2
    //   + c3 / n^3

    const double one_percent =
        evaluate_response_surface(
            -3.89644,
            -10.9519,
            -33.527,
            0.0,
            n
        );

    const double five_percent =
        evaluate_response_surface(
            -3.33613,
            -6.1101,
            -6.823,
            0.0,
            n
        );

    const double ten_percent =
        evaluate_response_surface(
            -3.04445,
            -4.2412,
            -2.720,
            0.0,
            n
        );

    if (!std::isfinite(one_percent) ||
        !std::isfinite(five_percent) ||
        !std::isfinite(ten_percent)) {

        throw std::overflow_error(
            "MacKinnon critical-value calculation "
            "produced non-finite output"
        );
    }

    return MacKinnonCriticalValues{
        one_percent,
        five_percent,
        ten_percent
    };
}

} // namespace quant::math