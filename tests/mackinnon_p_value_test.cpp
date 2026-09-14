#include "quant/math/mackinnon.hpp"

#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {

bool approximately_equal(
    double lhs,
    double rhs,
    double tolerance
) {
    return std::abs(lhs - rhs) <= tolerance;
}

} // namespace

int main() {
    using quant::math::mackinnon_cointegration_p_value;

    constexpr std::size_t observations = 100;

    const double strong_statistic =
        -4.126339428;

    const double strong_p_value =
        mackinnon_cointegration_p_value(
            strong_statistic,
            observations
        );

    assert(
        strong_p_value >= 0.0 &&
        strong_p_value <= 1.0
    );

    assert(
        approximately_equal(
            strong_p_value,
            0.0047039157,
            1e-8
        )
    );

    /*
     * Left-tail monotonicity.
     */
    const double p1 =
        mackinnon_cointegration_p_value(
            -3.0,
            observations
        );

    const double p2 =
        mackinnon_cointegration_p_value(
            -4.0,
            observations
        );

    const double p3 =
        mackinnon_cointegration_p_value(
            -5.0,
            observations
        );

    assert(p3 < p2);
    assert(p2 < p1);

    /*
     * Lower and upper support boundaries.
     */
    assert(
        mackinnon_cointegration_p_value(
            -100.0,
            observations
        ) == 0.0
    );

    assert(
        mackinnon_cointegration_p_value(
            1.0,
            observations
        ) == 1.0
    );

    /*
     * NaN statistic must be rejected.
     */
    bool threw = false;

    try {
        (void)mackinnon_cointegration_p_value(
            std::numeric_limits<double>::quiet_NaN(),
            observations
        );
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    /*
     * Invalid observation count must be rejected.
     */
    threw = false;

    try {
        (void)mackinnon_cointegration_p_value(
            -4.0,
            1
        );
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    return 0;
}