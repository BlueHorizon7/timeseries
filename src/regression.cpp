#include "quant/math/regression.hpp"
#include "quant/math/statistics.hpp"

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

    for (std::size_t i = 0; i < x.size(); ++i) {
        if (x[i].timestamp != y[i].timestamp) {
            throw std::invalid_argument(
                "OLS requires aligned timestamps"
            );
        }
    }

    const double variance_x = variance(x);

    if (variance_x == 0.0) {
        throw std::domain_error(
            "OLS undefined when independent variable has zero variance"
        );
    }

    const double beta =
        covariance(x, y) / variance_x;

    const double alpha =
        mean(y) - beta * mean(x);

    return LinearRegression{
        alpha,
        beta
    };
}

} // namespace quant::math