#include "quant/math/regression.hpp"

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
    x.add(Observation{4000, 4.0});

    // y = 2x + 3
    y.add(Observation{1000, 5.0});
    y.add(Observation{2000, 7.0});
    y.add(Observation{3000, 9.0});
    y.add(Observation{4000, 11.0});

    const auto result =
        quant::math::ordinary_least_squares(x, y);

    assert(
        std::abs(result.slope - 2.0) < 1e-12
    );

    assert(
        std::abs(result.intercept - 3.0) < 1e-12
    );
}