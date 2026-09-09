#include "quant/math/regression.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::math {

LinearRegression ordinary_least_squares(
    const Series& x,
    const Series& y
) {
    if (x.size() != y.size()) {
        throw std::invalid_argument(
            "OLS requires equal-length series"
        );
    }

    if (x.size() < 2) {
        throw std::invalid_argument(
            "OLS requires at least two observations"
        );
    }

    /*
     * Verify temporal alignment.
     */
    for (std::size_t i = 0; i < x.size(); ++i) {
        if (x[i].timestamp != y[i].timestamp) {
            throw std::invalid_argument(
                "OLS requires aligned timestamps"
            );
        }

        if (!std::isfinite(x[i].value) ||
            !std::isfinite(y[i].value)) {
            throw std::domain_error(
                "OLS requires finite observations"
            );
        }
    }

    /*
     * First pass:
     *
     *     mean_x
     *     mean_y
     *
     * Use an incremental mean to reduce loss of precision.
     */
    double mean_x = 0.0;
    double mean_y = 0.0;

    for (std::size_t i = 0; i < x.size(); ++i) {
        const double k =
            static_cast<double>(i + 1);

        mean_x +=
            (x[i].value - mean_x) / k;

        mean_y +=
            (y[i].value - mean_y) / k;
    }

    /*
     * Second pass:
     *
     *     Sxx = Σ(x_i - x̄)^2
     *     Sxy = Σ(x_i - x̄)(y_i - ȳ)
     */
    double sxx = 0.0;
    double sxy = 0.0;

    for (std::size_t i = 0; i < x.size(); ++i) {
        const double dx =
            x[i].value - mean_x;

        const double dy =
            y[i].value - mean_y;

        sxx += dx * dx;
        sxy += dx * dy;
    }

    if (!std::isfinite(sxx) ||
        !std::isfinite(sxy)) {
        throw std::overflow_error(
            "OLS accumulation overflow"
        );
    }

    if (sxx == 0.0) {
        throw std::domain_error(
            "OLS undefined when independent variable has zero variance"
        );
    }

    const double slope =
        sxy / sxx;

    const double intercept =
        mean_y - slope * mean_x;

    if (!std::isfinite(intercept) ||
        !std::isfinite(slope)) {
        throw std::overflow_error(
            "OLS produced a non-finite result"
        );
    }

    return LinearRegression{
        intercept,
        slope
    };
}

} // namespace quant::math