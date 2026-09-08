#include "quant/math/stationarity.hpp"

#include <cassert>
#include <cmath>

int main() {
    using quant::math::Observation;
    using quant::math::Series;

    Series series;

    // S_t = 2 + 0.5 S_(t-1)
    //
    // Starting from S_0 = 0:
    // 0, 2, 3, 3.5, 3.75, 3.875

    series.add(Observation{1000, 0.0});
    series.add(Observation{2000, 2.0});
    series.add(Observation{3000, 3.0});
    series.add(Observation{4000, 3.5});
    series.add(Observation{5000, 3.75});
    series.add(Observation{6000, 3.875});

    const auto result =
        quant::math::fit_ar1(series);

    assert(
        std::abs(result.intercept - 2.0) < 1e-12
    );

    assert(
        std::abs(result.phi - 0.5) < 1e-12
    );

    return 0;
}