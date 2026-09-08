#include "quant/math/cointegration.hpp"

#include <cassert>
#include <cmath>

int main() {
    using quant::math::CointegrationDecision;
    using quant::math::Observation;
    using quant::math::Series;

    Series x;
    Series y;

    const double stationary_component[] = {
         0.10, -0.20,  0.15, -0.10,  0.05,
        -0.15,  0.20, -0.05,  0.10, -0.10,
         0.15, -0.20,  0.05, -0.10,  0.20,
        -0.15,  0.10, -0.05,  0.15, -0.20,
         0.10, -0.10,  0.20, -0.15,  0.05,
        -0.05,  0.15, -0.10,  0.10, -0.15,
         0.20, -0.05,  0.10, -0.20,  0.15,
        -0.10,  0.05, -0.15,  0.20, -0.05
    };

    double x_value = 100.0;

    for (std::size_t i = 0;
         i < sizeof(stationary_component) /
             sizeof(stationary_component[0]);
         ++i) {

        // Non-stationary component.
        x_value +=
            (i % 3 == 0 ? 1.0 : -0.3);

        x.add(
            Observation{
                static_cast<std::int64_t>(i),
                x_value
            }
        );

        const double u =
            stationary_component[i];

        // Y = 2X + 3 + stationary noise.
        y.add(
            Observation{
                static_cast<std::int64_t>(i),
                2.0 * x_value + 3.0 + u
            }
        );
    }

    const auto result =
        quant::math::engle_granger(x, y);

    assert(
        result.spread.size() == x.size()
    );

    assert(
        std::abs(result.regression.slope - 2.0) < 0.01
    );

    assert(
        std::abs(result.regression.intercept - 3.0) < 0.5
    );

    assert(
        result.adf_statistic < 0.0
    );

    assert(
        result.decision ==
        CointegrationDecision::Cointegrated
    );

    return 0;
}