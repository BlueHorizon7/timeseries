#include "quant/math/rolling_spread.hpp"

#include <cassert>
#include <cmath>

int main() {
    using quant::math::Observation;
    using quant::math::Series;

    Series x;
    Series y;

    /*
        Historical relationship:

            Y = 2X + 1

        for observations 0, 1, 2.
    */

    x.add(Observation{1000, 1.0});
    y.add(Observation{1000, 3.0});

    x.add(Observation{2000, 2.0});
    y.add(Observation{2000, 5.0});

    x.add(Observation{3000, 3.0});
    y.add(Observation{3000, 7.0});

    /*
        At t = 3, relationship breaks.

        Historical model predicts:

            2 * 4 + 1 = 9

        Actual Y = 14

        Therefore spread = 5.
    */

    x.add(Observation{4000, 4.0});
    y.add(Observation{4000, 14.0});

    /*
        Add another observation so that we obtain
        a second rolling spread value.
    */

    x.add(Observation{5000, 5.0});
    y.add(Observation{5000, 16.0});

    const Series spread =
        quant::math::rolling_spread(
            x,
            y,
            3
        );

    assert(spread.size() == 2);

    /*
        First model uses observations 0, 1, 2:

            alpha = 1
            beta  = 2

        At t = 3:

            predicted = 9
            actual    = 14

            spread = 5
    */

    assert(
        spread[0].timestamp == 4000
    );

    assert(
        std::abs(
            spread[0].value - 5.0
        ) < 1e-12
    );

    /*
        The second model is re-estimated using
        observations 1, 2, 3.

        We do not hard-code its exact residual here,
        but it must correspond to timestamp 5000.
    */

    assert(
        spread[1].timestamp == 5000
    );

    return 0;
}