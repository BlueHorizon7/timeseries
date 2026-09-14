#include "quant/math/regression.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace quant::math {

namespace {

struct RunningMean {
    long double value{0.0L};
    std::size_t count{0};

    void add(long double observation) {
        ++count;

        const long double delta =
            observation - value;

        value +=
            delta /
            static_cast<long double>(count);
    }
};

} // namespace

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
        Verify temporal alignment and finiteness.
    */
    for (std::size_t i = 0;
         i < x.size();
         ++i) {

        if (x[i].timestamp !=
            y[i].timestamp) {

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
        First pass.

        Use long double for the running means so that the
        accumulation has more precision than the input data.
    */
    RunningMean mean_x;
    RunningMean mean_y;

    for (std::size_t i = 0;
         i < x.size();
         ++i) {

        mean_x.add(
            static_cast<long double>(
                x[i].value
            )
        );

        mean_y.add(
            static_cast<long double>(
                y[i].value
            )
        );
    }

    /*
        Second pass.

            Sxx = Σ (x_i - x̄)^2
            Sxy = Σ (x_i - x̄)(y_i - ȳ)

        Again accumulate in long double.
    */
    long double sxx = 0.0L;
    long double sxy = 0.0L;

    for (std::size_t i = 0;
         i < x.size();
         ++i) {

        const long double dx =
            static_cast<long double>(
                x[i].value
            ) -
            mean_x.value;

        const long double dy =
            static_cast<long double>(
                y[i].value
            ) -
            mean_y.value;

        sxx +=
            dx * dx;

        sxy +=
            dx * dy;
    }

    if (!std::isfinite(sxx) ||
        !std::isfinite(sxy)) {

        throw std::overflow_error(
            "OLS accumulation overflow"
        );
    }

    /*
        Exact zero variance is undefined.
    */
    if (sxx == 0.0L) {
        throw std::domain_error(
            "OLS undefined when independent variable "
            "has zero variance"
        );
    }

    const long double slope_ld =
        sxy / sxx;

    const long double intercept_ld =
        mean_y.value -
        slope_ld * mean_x.value;

    const double slope =
        static_cast<double>(
            slope_ld
        );

    const double intercept =
        static_cast<double>(
            intercept_ld
        );

    if (!std::isfinite(slope) ||
        !std::isfinite(intercept)) {

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