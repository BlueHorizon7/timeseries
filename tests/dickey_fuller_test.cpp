#include "quant/math/dickey_fuller.hpp"

#include <cassert>
#include <cmath>

int main() {
    using quant::math::Observation;
    using quant::math::Series;

    Series series;

    /*
        Construct a stationary AR(1)-like process:

        S_t = 2 + 0.5 S_(t-1) + error_t

        The disturbances are deterministic so that
        this remains a reproducible unit test.
    */

    series.add(Observation{1000, 0.0});
    series.add(Observation{2000, 2.1});
    series.add(Observation{3000, 3.0});
    series.add(Observation{4000, 3.55});
    series.add(Observation{5000, 3.70});
    series.add(Observation{6000, 3.95});
    series.add(Observation{7000, 3.80});
    series.add(Observation{8000, 4.10});
    series.add(Observation{9000, 3.90});
    series.add(Observation{10000, 4.05});

    const auto result =
        quant::math::dickey_fuller(series);

    /*
        Since the process is mean-reverting, gamma should
        be negative:

            gamma = phi - 1

        For phi < 1:

            gamma < 0
    */

    assert(result.gamma < 0.0);

    assert(result.standard_error > 0.0);

    /*
        A mean-reverting process should produce a
        negative Dickey-Fuller statistic.
    */

    assert(result.statistic < 0.0);

    return 0;
}