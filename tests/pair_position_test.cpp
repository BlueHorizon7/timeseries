#include "quant/portfolio/pair_position.hpp"

#include <cassert>
#include <cmath>
#include <stdexcept>

int main() {
    using quant::portfolio::PairPosition;

    /*
        Long spread:

            beta = 1.5
            gross = 100000

            X = -60000
            Y = +40000
    */

    const PairPosition long_position =
        quant::portfolio::construct_pair_position(
            1,
            1.5,
            100000.0
        );

    assert(
        std::abs(
            long_position.x_notional + 60000.0
        ) < 1e-12
    );

    assert(
        std::abs(
            long_position.y_notional - 40000.0
        ) < 1e-12
    );

    /*
        Short spread reverses both legs.
    */

    const PairPosition short_position =
        quant::portfolio::construct_pair_position(
            -1,
            1.5,
            100000.0
        );

    assert(
        std::abs(
            short_position.x_notional - 60000.0
        ) < 1e-12
    );

    assert(
        std::abs(
            short_position.y_notional + 40000.0
        ) < 1e-12
    );

    /*
        Flat position.
    */

    const PairPosition flat_position =
        quant::portfolio::construct_pair_position(
            0,
            1.5,
            100000.0
        );

    assert(flat_position.x_notional == 0.0);
    assert(flat_position.y_notional == 0.0);

    /*
        Gross exposure must equal requested
        gross notional.
    */

    const double gross =
        std::abs(long_position.x_notional) +
        std::abs(long_position.y_notional);

    assert(
        std::abs(gross - 100000.0) < 1e-12
    );

    /*
        Invalid signal.
    */

    bool threw = false;

    try {
        static_cast<void>(
            quant::portfolio::construct_pair_position(
                2,
                1.5,
                100000.0
            )
        );
    }
    catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    /*
        Negative notional.
    */

    threw = false;

    try {
        static_cast<void>(
            quant::portfolio::construct_pair_position(
                1,
                1.5,
                -100.0
            )
        );
    }
    catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    return 0;
}