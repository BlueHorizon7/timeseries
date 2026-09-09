#include "quant/portfolio/pnl.hpp"

#include <cassert>
#include <cmath>
#include <stdexcept>

int main() {
    using quant::portfolio::PairPosition;

    const PairPosition position{
        -60000.0,
        40000.0
    };

    const auto result =
        quant::portfolio::calculate_pnl(
            position,
            100.0,
            105.0,
            200.0,
            210.0
        );

    assert(
        std::abs(
            result.x_pnl + 3000.0
        ) < 1e-12
    );

    assert(
        std::abs(
            result.y_pnl - 2000.0
        ) < 1e-12
    );

    assert(
        std::abs(
            result.total_pnl + 1000.0
        ) < 1e-12
    );

    /*
        Flat position must produce zero P&L.
    */

    const PairPosition flat{
        0.0,
        0.0
    };

    const auto flat_result =
        quant::portfolio::calculate_pnl(
            flat,
            100.0,
            200.0,
            100.0,
            50.0
        );

    assert(flat_result.x_pnl == 0.0);
    assert(flat_result.y_pnl == 0.0);
    assert(flat_result.total_pnl == 0.0);

    /*
        Invalid prices must fail.
    */

    bool threw = false;

    try {
        static_cast<void>(
            quant::portfolio::calculate_pnl(
                position,
                0.0,
                105.0,
                200.0,
                210.0
            )
        );
    }
    catch (const std::domain_error&) {
        threw = true;
    }

    assert(threw);

    return 0;
}