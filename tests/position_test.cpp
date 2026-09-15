#include "quant/execution/position.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    quant::execution::Position position{};

    position.symbol = "TEST";

    /*
        Open:

            Buy 100 @ 100
            Commission = 1

        Realized PnL:
            -1
    */
    quant::execution::apply_position_fill(
        position,
        true,
        100.0,
        100.0,
        1.0,
        1
    );

    assert(
        position.quantity ==
        100.0
    );

    assert(
        position.average_entry_price ==
        100.0
    );

    assert(
        std::abs(
            position.realized_pnl -
            (-1.0)
        ) < 1e-12
    );

    /*
        Mark:

            100 shares @ 110

        Unrealized PnL:
            100 * (110 - 100) = 1000
    */
    quant::execution::mark_position(
        position,
        110.0,
        2
    );

    assert(
        std::abs(
            position.unrealized_pnl -
            1000.0
        ) < 1e-12
    );

    /*
        Partial close:

            Sell 40 @ 120
            Commission = 2

        Gross realized PnL:
            40 * (120 - 100)
            = 800

        Net realized PnL accumulated:
            -1 + 800 - 2
            = 797
    */
    quant::execution::apply_position_fill(
        position,
        false,
        40.0,
        120.0,
        2.0,
        3
    );

    assert(
        position.quantity ==
        60.0
    );

    assert(
        std::abs(
            position.realized_pnl -
            797.0
        ) < 1e-12
    );

    assert(
        position.average_entry_price ==
        100.0
    );

    /*
        Close remaining position:

            Sell 60 @ 90
            Commission = 1

        Incremental realized PnL:
            60 * (90 - 100) - 1
            = -601

        Final accumulated realized PnL:
            797 - 601
            = 196
    */
    quant::execution::apply_position_fill(
        position,
        false,
        60.0,
        90.0,
        1.0,
        4
    );

    assert(
        position.quantity ==
        0.0
    );

    assert(
        position.average_entry_price ==
        0.0
    );

    assert(
        std::abs(
            position.realized_pnl -
            196.0
        ) < 1e-12
    );

    /*
        Flat position has no unrealized PnL.
    */
    assert(
        std::abs(
            position.unrealized_pnl
        ) < 1e-12
    );

    /*
        Reversal:

            Sell 50 @ 100
            -> short 50 @ 100
    */
    quant::execution::apply_position_fill(
        position,
        false,
        50.0,
        100.0,
        0.0,
        5
    );

    assert(
        position.quantity ==
        -50.0
    );

    assert(
        position.average_entry_price ==
        100.0
    );

    /*
        Reverse through zero:

            Buy 75 @ 90

        First 50 closes the short:

            50 * (100 - 90)
            = 500

        Remaining 25 opens a new long:

            +25 @ 90

        Therefore the position becomes:
            quantity = +25
            average_entry_price = 90
    */
    quant::execution::apply_position_fill(
        position,
        true,
        75.0,
        90.0,
        0.0,
        6
    );

    assert(
        position.quantity ==
        25.0
    );

    assert(
        position.average_entry_price ==
        90.0
    );

    /*
        Previously realized PnL was 196.

        Closing the 50-share short realizes:

            500

        Final realized PnL:
            196 + 500
            = 696
    */
    assert(
        std::abs(
            position.realized_pnl -
            696.0
        ) < 1e-12
    );

    /*
        Mark the new long position.
    */
    quant::execution::mark_position(
        position,
        100.0,
        7
    );

    assert(
        std::abs(
            position.unrealized_pnl -
            250.0
        ) < 1e-12
    );

    /*
        Timestamp regression must be rejected.
    */
    {
        bool threw = false;

        try {
            quant::execution::mark_position(
                position,
                101.0,
                6
            );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        Invalid quantity must be rejected.
    */
    {
        bool threw = false;

        try {
            quant::execution::apply_position_fill(
                position,
                true,
                0.0,
                100.0,
                0.0,
                8
            );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        Invalid price must be rejected.
    */
    {
        bool threw = false;

        try {
            quant::execution::apply_position_fill(
                position,
                true,
                1.0,
                0.0,
                0.0,
                8
            );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        Negative commission must be rejected.
    */
    {
        bool threw = false;

        try {
            quant::execution::apply_position_fill(
                position,
                true,
                1.0,
                100.0,
                -1.0,
                8
            );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    std::cout
        << "Position tests passed!\n";

    return 0;
}