#include "quant/math/cointegration.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>

int main() {
    quant::math::Series x;
    quant::math::Series y;

    constexpr std::size_t n = 500;

    x.reserve(n);
    y.reserve(n);

    double random_walk = 100.0;

    for (std::size_t i = 0;
         i < n;
         ++i) {

        /*
         * Deterministic pseudo-random-walk-like
         * integrated process.
         */
        const double innovation =
            std::sin(
                static_cast<double>(i) * 0.37
            );

        random_walk += innovation;

        /*
         * Stationary bounded disturbance.
         */
        const double noise =
            0.5 *
            std::sin(
                static_cast<double>(i) * 1.31
            );

        const double x_value =
            random_walk;

        const double y_value =
            4.0 +
            1.7 * x_value +
            noise;

        const std::int64_t timestamp =
            static_cast<std::int64_t>(
                i + 1
            );

        x.add(
            quant::math::Observation{
                timestamp,
                x_value
            }
        );

        y.add(
            quant::math::Observation{
                timestamp,
                y_value
            }
        );
    }

    const auto result =
        quant::math::engle_granger(
            x,
            y,
            0
        );

    assert(
        std::abs(
            result.regression.slope -
            1.7
        ) < 0.05
    );

    assert(
        result.decision ==
        quant::math::CointegrationDecision::
            Cointegrated
    );

    assert(
        result.adf_statistic <
        result.critical_values.five_percent
    );

    assert(
        result.observations == n
    );

    return 0;
}