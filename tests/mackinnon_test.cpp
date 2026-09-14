#include "quant/math/mackinnon.hpp"

#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {

bool approximately_equal(
    double lhs,
    double rhs,
    double tolerance = 1e-10
) {
    return
        std::abs(lhs - rhs) <= tolerance;
}

} // namespace

int main() {
    using quant::math::
        mackinnon_cointegration_critical_values;

    constexpr std::size_t n = 100;

    const auto critical =
        mackinnon_cointegration_critical_values(n);

    const double inverse_n =
        1.0 /
        static_cast<double>(n);

    const double expected_one =
        -3.89644
        - 10.9519 * inverse_n
        - 33.527 * inverse_n * inverse_n;

    const double expected_five =
        -3.33613
        - 6.1101 * inverse_n
        - 6.823 * inverse_n * inverse_n;

    const double expected_ten =
        -3.04445
        - 4.2412 * inverse_n
        - 2.720 * inverse_n * inverse_n;

    assert(
        approximately_equal(
            critical.one_percent,
            expected_one
        )
    );

    assert(
        approximately_equal(
            critical.five_percent,
            expected_five
        )
    );

    assert(
        approximately_equal(
            critical.ten_percent,
            expected_ten
        )
    );

    assert(
        critical.one_percent <
        critical.five_percent
    );

    assert(
        critical.five_percent <
        critical.ten_percent
    );

    /*
     * Continuous MacKinnon p-value.
     */
    const double p_value =
        quant::math::mackinnon_cointegration_p_value(
            -4.126339428,
            n
        );

    assert(
        p_value >= 0.0 &&
        p_value <= 1.0
    );

    assert(
        approximately_equal(
            p_value,
            0.0047039157,
            1e-8
        )
    );

    /*
     * More negative test statistics must produce
     * smaller left-tail p-values.
     */
    const double weaker_p_value =
        quant::math::mackinnon_cointegration_p_value(
            -3.0,
            n
        );

    assert(
        p_value <
        weaker_p_value
    );

    /*
     * Invalid statistic.
     */
    bool threw = false;

    try {
        (void)
            quant::math::mackinnon_cointegration_p_value(
                std::numeric_limits<double>::quiet_NaN(),
                n
            );
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    /*
     * Invalid observation count.
     */
    threw = false;

    try {
        (void)
            quant::math::mackinnon_cointegration_p_value(
                -4.0,
                1
            );
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    return 0;
}