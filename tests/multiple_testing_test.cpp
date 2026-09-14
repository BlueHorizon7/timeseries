#include "quant/statistics/multiple_testing.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

int main() {

    using quant::statistics::benjamini_hochberg;

    /*
        Four clearly significant hypotheses plus
        two non-significant ones.
    */
    const std::vector<double> p_values{
        0.001,
        0.004,
        0.010,
        0.020,
        0.400,
        0.800
    };

    const auto result =
        benjamini_hochberg(
            p_values,
            0.05
        );

    assert(
        result.adjusted_p_values.size() ==
        p_values.size()
    );

    assert(
        result.rejected.size() ==
        p_values.size()
    );

    /*
        BH adjusted p-values must be within [0,1].
    */
    for (const double p :
         result.adjusted_p_values) {

        assert(
            std::isfinite(p)
        );

        assert(
            p >= 0.0 &&
            p <= 1.0
        );
    }

    /*
        The first four should survive FDR control.
    */
    assert(result.rejected[0]);
    assert(result.rejected[1]);
    assert(result.rejected[2]);
    assert(result.rejected[3]);

    assert(!result.rejected[4]);
    assert(!result.rejected[5]);

    assert(
        result.number_rejected == 4
    );

    /*
        Test input-order preservation.

        Same statistical values, different order.
    */
    const std::vector<double> reordered{
        0.800,
        0.001,
        0.400,
        0.010,
        0.004,
        0.020
    };

    const auto reordered_result =
        benjamini_hochberg(
            reordered,
            0.05
        );

    assert(!reordered_result.rejected[0]);
    assert(reordered_result.rejected[1]);
    assert(!reordered_result.rejected[2]);
    assert(reordered_result.rejected[3]);
    assert(reordered_result.rejected[4]);
    assert(reordered_result.rejected[5]);

    /*
        Equal p-values must behave deterministically.
    */
    const std::vector<double> ties{
        0.01,
        0.01,
        0.01,
        0.01
    };

    const auto tie_result =
        benjamini_hochberg(
            ties,
            0.05
        );

    for (const bool value :
         tie_result.rejected) {

        assert(value);
    }

    /*
        Invalid FDR.
    */
    bool threw = false;

    try {
        (void)benjamini_hochberg(
            p_values,
            0.0
        );
    }
    catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    /*
        Invalid p-value.
    */
    threw = false;

    try {
        (void)benjamini_hochberg(
            std::vector<double>{
                0.01,
                1.2
            },
            0.05
        );
    }
    catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    /*
        Empty input.
    */
    threw = false;

    try {
        (void)benjamini_hochberg(
            {},
            0.05
        );
    }
    catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    std::cout
        << "Multiple-testing tests passed!"
        << std::endl;

    return 0;
}