#include "quant/math/mackinnon.hpp"

#include <cmath>
#include <algorithm>
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

double standard_normal_cdf(double x) {
    return
        0.5 *
        std::erfc(
            -x / std::sqrt(2.0)
        );
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

    // MacKinnon (2010) response-surface coefficients.
    //
    // N = 2:
    // two I(1) series.
    //
    // Deterministic specification:
    // constant, no time trend.
    //
    // critical value =
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

        throw std::runtime_error(
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

double
mackinnon_cointegration_p_value(
    double statistic,
    std::size_t observations
) {
    if (observations < 2) {
        throw std::invalid_argument(
            "MacKinnon p-value requires "
            "at least two observations"
        );
    }

    if (!std::isfinite(statistic)) {
        throw std::invalid_argument(
            "MacKinnon p-value requires "
            "a finite test statistic"
        );
    }

    /*
     * MacKinnon numerical-distribution approximation.
     *
     * Engle-Granger case:
     *   N = 2
     *   constant
     *   no time trend
     *
     * These are the MacKinnon (1996) response-surface
     * coefficients used for the continuous asymptotic
     * distribution function.
     *
     * For N = 2, constant:
     *
     *   tau_star = -2.62
     *   tau_min  = -18.86
     *   tau_max  =  0.92
     *
     * For the left tail relevant to cointegration:
     *
     * p = Phi(
     *       d0
     *     + d1 * tau
     *     + d2 * tau^2
     * )
     *
     * with:
     *
     *   d0 = 2.92
     *   d1 = 1.5012
     *   d2 = 0.039796
     *
     * The finite-sample observation count is validated above,
     * but this particular p-value response surface is
     * asymptotic; sample-size dependence is handled by the
     * separate MacKinnon (2010) critical-value surface.
     */

    constexpr double tau_min = -18.86;
    constexpr double tau_max = 0.92;
    constexpr double tau_star = -2.62;

    if (statistic <= tau_min) {
        return 0.0;
    }

    if (statistic >= tau_max) {
        return 1.0;
    }

    /*
     * For the Engle-Granger statistic, the relevant region
     * for conventional cointegration evidence is the left
     * tail, which uses the small-p approximation.
     */
    if (statistic <= tau_star) {
        constexpr double d0 = 2.92;
        constexpr double d1 = 1.5012;
        constexpr double d2 = 3.9796e-2;

        const double transformed =
            d0 +
            d1 * statistic +
            d2 * statistic * statistic;

        const double p_value =
            standard_normal_cdf(transformed);

        if (!std::isfinite(p_value)) {
            throw std::runtime_error(
                "MacKinnon p-value calculation "
                "produced non-finite output"
            );
        }

        return
            std::clamp(
                p_value,
                0.0,
                1.0
            );
    }

    /*
     * For the remainder of the support, use the larger-p
     * approximation.
     *
     * N = 2, constant/no trend:
     *
     *   p = Phi(
     *         d0
     *       + d1 * tau
     *       + d2 * tau^2
     *       + d3 * tau^3
     *       )
     *
     * where the MacKinnon coefficients are stored below
     * after applying the original scaling convention.
     *
     * Raw coefficients:
     *
     *   [2.1945, 6.4695, -2.9198, -4.2377]
     *
     * scaling:
     *
     *   [1, 0.1, 0.1, 0.01]
     */

    constexpr double d0 = 2.1945;
    constexpr double d1 = 0.64695;
    constexpr double d2 = -0.29198;
    constexpr double d3 = -0.042377;

    const double transformed =
        d0 +
        d1 * statistic +
        d2 * statistic * statistic +
        d3 * statistic * statistic * statistic;

    const double p_value =
        standard_normal_cdf(transformed);

    if (!std::isfinite(p_value)) {
        throw std::runtime_error(
            "MacKinnon p-value calculation "
            "produced non-finite output"
        );
    }

    return
        std::clamp(
            p_value,
            0.0,
            1.0
        );
}

} // namespace quant::math