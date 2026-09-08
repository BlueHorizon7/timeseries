#include "quant/math/residuals.hpp"

#include <cassert>
#include <cmath>
#include <stdexcept>

int main() {
    using quant::math::LinearRegression;
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

    const LinearRegression model{
        3.0,
        2.0
    };

    const Series result =
        quant::math::residuals(x, y, model);

    assert(result.size() == 4);

    for (std::size_t i = 0; i < result.size(); ++i) {
        assert(
            std::abs(result[i].value) < 1e-12
        );

        assert(
            result[i].timestamp == y[i].timestamp
        );
    }

    // Test a non-zero residual series.
    Series y2;

    y2.add(Observation{1000, 6.0});
    y2.add(Observation{2000, 6.0});
    y2.add(Observation{3000, 10.0});
    y2.add(Observation{4000, 10.0});

    const Series result2 =
        quant::math::residuals(x, y2, model);

    assert(
        std::abs(result2[0].value - 1.0) < 1e-12
    );

    assert(
        std::abs(result2[1].value - (-1.0)) < 1e-12
    );

    assert(
        std::abs(result2[2].value - 1.0) < 1e-12
    );

    assert(
        std::abs(result2[3].value - (-1.0)) < 1e-12
    );

    // Mismatched lengths must fail.
    Series short_y;

    short_y.add(Observation{1000, 5.0});
    short_y.add(Observation{2000, 7.0});

    bool threw = false;

    try {
        static_cast<void>(
            quant::math::residuals(x, short_y, model)
        );
    }
    catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    // Misaligned timestamps must fail.
    Series misaligned_y;

    misaligned_y.add(Observation{1000, 5.0});
    misaligned_y.add(Observation{2000, 7.0});
    misaligned_y.add(Observation{3500, 9.0});
    misaligned_y.add(Observation{4000, 11.0});

    threw = false;

    try {
        static_cast<void>(
            quant::math::residuals(x, misaligned_y, model)
        );
    }
    catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    return 0;
}