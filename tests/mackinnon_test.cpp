#include "quant/math/mackinnon.hpp"

#include <cassert>
#include <cmath>

namespace {

bool approximately_equal(
    double lhs,
    double rhs,
    double tolerance = 1e-10
) {
    return
        std::abs(lhs - rhs) <= tolerance;
}

}

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

    return 0;
}