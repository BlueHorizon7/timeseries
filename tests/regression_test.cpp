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

        /*
        Large-offset data.

        The true relationship is:

            y = 3x + 7

        but x is centered around a very large value.
        This checks that the centered implementation
        does not lose the slope because of the offset.
    */
    Series large_x;
    Series large_y;

    constexpr double base =
        1.0e12;

    for (std::size_t i = 0;
         i < 100;
         ++i) {

        const double xv =
            base +
            static_cast<double>(i);

        const double yv =
            3.0 * xv +
            7.0;

        large_x.add(
            Observation{
                static_cast<std::int64_t>(i + 1),
                xv
            }
        );

        large_y.add(
            Observation{
                static_cast<std::int64_t>(i + 1),
                yv
            }
        );
    }

    const auto large_result =
        quant::math::ordinary_least_squares(
            large_x,
            large_y
        );

    assert(
        
        std::abs(
            large_result.slope -
            3.0
        ) < 1e-12
    );

    assert(
        std::isfinite(
            large_result.intercept
        )
    );
}