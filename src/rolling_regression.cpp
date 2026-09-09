#include "quant/math/rolling_regression.hpp"

#include <stdexcept>
#include <vector>

namespace quant::math {

std::vector<RollingRegressionResult>
rolling_ols(
    const Series& x,
    const Series& y,
    std::size_t window
) {
    if (x.size() != y.size()) {
        throw std::invalid_argument(
            "Rolling OLS requires equal-length series"
        );
    }

    if (window < 2) {
        throw std::invalid_argument(
            "Rolling OLS requires a window of at least two observations"
        );
    }

    if (x.size() <= window) {
        throw std::invalid_argument(
            "Rolling OLS requires observations beyond the estimation window"
        );
    }

    for (std::size_t i = 0; i < x.size(); ++i) {
        if (x[i].timestamp != y[i].timestamp) {
            throw std::invalid_argument(
                "Rolling OLS requires aligned timestamps"
            );
        }
    }

    std::vector<RollingRegressionResult> results;

    results.reserve(x.size() - window);

    /*
        At timestamp t, estimate using:

            [t - window, ..., t - 1]

        and associate the resulting model with t.

        Therefore observation t itself is NOT used
        to estimate the model used at t.
    */

    for (std::size_t t = window; t < x.size(); ++t) {
        Series x_window;
        Series y_window;

        x_window.reserve(window);
        y_window.reserve(window);

        for (std::size_t i = t - window; i < t; ++i) {
            x_window.add(x[i]);
            y_window.add(y[i]);
        }

        const LinearRegression model =
            ordinary_least_squares(
                x_window,
                y_window
            );

        results.push_back(
            RollingRegressionResult{
                t,
                model
            }
        );
    }

    return results;
}

} // namespace quant::math