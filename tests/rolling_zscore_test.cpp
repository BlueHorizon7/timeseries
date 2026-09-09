#include "quant/math/rolling_zscore.hpp"

#include <cassert>
#include <cmath>
#include <stdexcept>

int main() {
    using quant::math::Observation;
    using quant::math::Series;

    Series series;

    series.add(Observation{1000, 1.0});
    series.add(Observation{2000, 2.0});
    series.add(Observation{3000, 3.0});

    // Current observation.
    series.add(Observation{4000, 5.0});

    const Series result =
        quant::math::rolling_zscore(
            series,
            3
        );

    assert(result.size() == 1);

    assert(
        result[0].timestamp == 4000
    );

    assert(
        std::abs(
            result[0].value - 3.0
        ) < 1e-12
    );

    /*
        The current observation must not enter
        the estimation window.

        If it did, the result would not be 3.
    */

    // Zero-variance window must fail.

    Series constant;

    constant.add(Observation{1000, 5.0});
    constant.add(Observation{2000, 5.0});
    constant.add(Observation{3000, 5.0});
    constant.add(Observation{4000, 6.0});

    bool threw = false;

    try {
        static_cast<void>(
            quant::math::rolling_zscore(
                constant,
                3
            )
        );
    }
    catch (const std::domain_error&) {
        threw = true;
    }

    assert(threw);

    return 0;
}