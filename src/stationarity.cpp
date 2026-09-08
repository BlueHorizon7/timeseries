#include "quant/math/stationarity.hpp"
#include "quant/math/regression.hpp"

#include <stdexcept>

namespace quant::math {

AR1Result fit_ar1(const Series& series) {
    if (series.size() < 3) {
        throw std::invalid_argument(
            "AR(1) regression requires at least three observations"
        );
    }

    Series x;
    Series y;

    x.reserve(series.size() - 1);
    y.reserve(series.size() - 1);

    for (std::size_t i = 1; i < series.size(); ++i) {
        x.add(Observation{
            series[i].timestamp,
            series[i - 1].value
        });

        y.add(Observation{
            series[i].timestamp,
            series[i].value
        });
    }

    const LinearRegression model =
        ordinary_least_squares(x, y);

    return AR1Result{
        model.intercept,
        model.slope
    };
}

} // namespace quant::math