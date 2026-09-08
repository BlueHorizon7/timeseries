#include "quant/math/dickey_fuller.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::math {

DickeyFullerResult dickey_fuller(
    const Series& series
) {
    if (series.size() < 3) {
        throw std::invalid_argument(
            "Dickey-Fuller test requires at least three observations"
        );
    }

    // Regression:
    //
    // ΔS_t = intercept + gamma * S_(t-1) + error_t
    //
    // where:
    //
    // ΔS_t = S_t - S_(t-1)

    const std::size_t n = series.size() - 1;

    double sum_x = 0.0;
    double sum_y = 0.0;

    for (std::size_t i = 1; i < series.size(); ++i) {
        const double x = series[i - 1].value;
        const double y =
            series[i].value - series[i - 1].value;

        sum_x += x;
        sum_y += y;
    }

    const double mean_x =
        sum_x / static_cast<double>(n);

    const double mean_y =
        sum_y / static_cast<double>(n);

    double sxx = 0.0;
    double sxy = 0.0;

    for (std::size_t i = 1; i < series.size(); ++i) {
        const double x = series[i - 1].value;
        const double y =
            series[i].value - series[i - 1].value;

        const double dx = x - mean_x;
        const double dy = y - mean_y;

        sxx += dx * dx;
        sxy += dx * dy;
    }

    if (sxx == 0.0) {
        throw std::domain_error(
            "Dickey-Fuller regression undefined when lagged series has zero variance"
        );
    }

    const double gamma = sxy / sxx;

    const double intercept =
        mean_y - gamma * mean_x;

    // Calculate residual sum of squares.

    double residual_sum_of_squares = 0.0;

    for (std::size_t i = 1; i < series.size(); ++i) {
        const double x = series[i - 1].value;
        const double y =
            series[i].value - series[i - 1].value;

        const double predicted =
            intercept + gamma * x;

        const double residual =
            y - predicted;

        residual_sum_of_squares +=
            residual * residual;
    }

    // Two estimated parameters:
    //
    // intercept
    // gamma
    //
    // Therefore residual degrees of freedom = n - 2.

    if (n <= 2) {
        throw std::invalid_argument(
            "Dickey-Fuller test requires more than three observations"
        );
    }

    const double residual_variance =
        residual_sum_of_squares /
        static_cast<double>(n - 2);

    const double standard_error =
        std::sqrt(residual_variance / sxx);

    if (standard_error == 0.0) {
        throw std::domain_error(
            "Dickey-Fuller statistic undefined with zero standard error"
        );
    }

    const double statistic =
        gamma / standard_error;

    return DickeyFullerResult{
        intercept,
        gamma,
        standard_error,
        statistic
    };
}

} // namespace quant::math