#include "quant/math/residuals.hpp"

#include <stdexcept>

namespace quant::math {

Series residuals(
    const Series& x,
    const Series& y,
    const LinearRegression& model
) {
    if (x.size() != y.size()) {
        throw std::invalid_argument(
            "Residuals require equal-length series"
        );
    }

    if (x.empty()) {
        throw std::invalid_argument(
            "Residuals require a non-empty series"
        );
    }

    for (std::size_t i = 0; i < x.size(); ++i) {
        if (x[i].timestamp != y[i].timestamp) {
            throw std::invalid_argument(
                "Residuals require aligned timestamps"
            );
        }
    }

    Series result;
    result.reserve(x.size());

    for (std::size_t i = 0; i < x.size(); ++i) {
        const double predicted =
            model.intercept +
            model.slope * x[i].value;

        const double residual =
            y[i].value - predicted;

        result.add(Observation{
            y[i].timestamp,
            residual
        });
    }

    return result;
}

} // namespace quant::math