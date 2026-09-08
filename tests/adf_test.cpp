#include "quant/math/adf.hpp"

#include <cassert>
#include <cmath>

int main() {
    using quant::math::DeterministicTerm;
    using quant::math::Observation;
    using quant::math::Series;

    Series series;

    /*
        Mean-reverting process approximately satisfying:

            S_t = 2 + 0.5 S_(t-1) + noise

        Deterministic disturbances make the test reproducible.
    */

    const double values[] = {
        0.0,
        2.1,
        2.8,
        3.4,
        3.8,
        3.7,
        4.2,
        3.9,
        4.1,
        3.8,
        4.0,
        3.7,
        4.1,
        3.9,
        4.2,
        3.8,
        4.0,
        3.9,
        4.1,
        3.8,
        4.0,
        3.9,
        4.1,
        3.8,
        4.0,
        3.9,
        4.1,
        3.8,
        4.0,
        3.9
    };

    for (std::size_t i = 0;
         i < sizeof(values) / sizeof(values[0]);
         ++i) {

        series.add(
            Observation{
                static_cast<std::int64_t>(i),
                values[i]
            }
        );
    }

    const auto result =
        quant::math::augmented_dickey_fuller(
            series,
            1,
            DeterministicTerm::Intercept
        );

    assert(result.lags == 1);

    assert(
        result.deterministic ==
        DeterministicTerm::Intercept
    );

    assert(result.standard_error > 0.0);

    /*
        Stationary / mean-reverting process:
        gamma should be negative.
    */

    assert(result.gamma < 0.0);

    /*
        Therefore the ADF statistic should also
        be negative.
    */

    assert(result.statistic < 0.0);

    return 0;
}