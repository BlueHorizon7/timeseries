#include "quant/math/rolling_regression.hpp"

#include <cassert>
#include <cmath>

int main() {
    using quant::math::Observation;
    using quant::math::Series;

    Series x;
    Series y;

    x.add(Observation{1000, 1.0});
    x.add(Observation{2000, 2.0});
    x.add(Observation{3000, 3.0});

    // Historical relationship:
    // Y = 2X
    y.add(Observation{1000, 2.0});
    y.add(Observation{2000, 4.0});
    y.add(Observation{3000, 6.0});

    // Relationship changes here:
    // Y = 5X
    x.add(Observation{4000, 4.0});
    y.add(Observation{4000, 20.0});

    x.add(Observation{5000, 5.0});
    y.add(Observation{5000, 25.0});

    const auto results =
        quant::math::rolling_ols(
            x,
            y,
            3
        );

    assert(results.size() == 2);

    /*
        Model associated with t = 3 must use
        observations [0, 1, 2] only.

            beta = 2

        If t = 3 had accidentally been included,
        beta would already be contaminated by the
        new relationship.
    */

    assert(
        results[0].timestamp_index == 3
    );

    assert(
        std::abs(
            results[0].model.slope - 2.0
        ) < 1e-12
    );

    /*
        Model at t = 4 uses observations [1, 2, 3].

        These contain two observations with beta = 2
        and one observation with beta = 5, so the
        resulting slope should lie between them.
    */

    assert(
    results[1].timestamp_index == 4
);

assert(
    std::abs(
        results[1].model.slope - 8.0
    ) < 1e-12
);

    return 0;
}