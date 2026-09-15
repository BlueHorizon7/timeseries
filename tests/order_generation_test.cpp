#include "quant/execution/order_generation.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    using namespace quant::execution;

    /*
        --------------------------------------------------------
        Open positions.
        --------------------------------------------------------
    */
    {
        const std::vector<TargetPosition> current{};

        const std::vector<TargetPosition> target{
            {"AAPL", 100.0},
            {"MSFT", -50.0}
        };

        const auto orders =
            generate_orders(
                current,
                target,
                100
            );

        assert(
            orders.size() == 2
        );

        /*
            Orders must be deterministic by symbol.
        */
        assert(
            orders[0].symbol == "AAPL"
        );

        assert(
            orders[1].symbol == "MSFT"
        );

        assert(
            orders[0].side ==
            OrderSide::Buy
        );

        assert(
            orders[1].side ==
            OrderSide::Sell
        );

        assert(
            orders[0].quantity == 100.0
        );

        assert(
            orders[1].quantity == 50.0
        );

        assert(
            orders[0].type ==
            OrderType::Market
        );

        assert(
            orders[1].type ==
            OrderType::Market
        );

        assert(
            orders[0].id == 0
        );

        assert(
            orders[1].id == 0
        );

        assert(
            orders[0].status ==
            OrderStatus::Created
        );

        assert(
            orders[1].status ==
            OrderStatus::Created
        );

        assert(
            orders[0].creation_timestamp ==
            100
        );

        assert(
            orders[1].creation_timestamp ==
            100
        );
    }

    /*
        --------------------------------------------------------
        Increase an existing long position.
        --------------------------------------------------------
    */
    {
        const std::vector<TargetPosition> current{
            {"AAPL", 100.0}
        };

        const std::vector<TargetPosition> target{
            {"AAPL", 150.0}
        };

        const auto orders =
            generate_orders(
                current,
                target,
                200
            );

        assert(
            orders.size() == 1
        );

        assert(
            orders[0].symbol == "AAPL"
        );

        assert(
            orders[0].side ==
            OrderSide::Buy
        );

        assert(
            std::abs(
                orders[0].quantity -
                50.0
            ) < 1e-12
        );
    }

    /*
        --------------------------------------------------------
        Reduce an existing long position.
        --------------------------------------------------------
    */
    {
        const std::vector<TargetPosition> current{
            {"AAPL", 100.0}
        };

        const std::vector<TargetPosition> target{
            {"AAPL", 40.0}
        };

        const auto orders =
            generate_orders(
                current,
                target,
                300
            );

        assert(
            orders.size() == 1
        );

        assert(
            orders[0].side ==
            OrderSide::Sell
        );

        assert(
            orders[0].quantity == 60.0
        );
    }

    /*
        --------------------------------------------------------
        Increase an existing short position.
        --------------------------------------------------------
    */
    {
        const std::vector<TargetPosition> current{
            {"AAPL", -100.0}
        };

        const std::vector<TargetPosition> target{
            {"AAPL", -150.0}
        };

        const auto orders =
            generate_orders(
                current,
                target,
                400
            );

        assert(
            orders.size() == 1
        );

        assert(
            orders[0].side ==
            OrderSide::Sell
        );

        assert(
            orders[0].quantity == 50.0
        );
    }

    /*
        --------------------------------------------------------
        Reduce an existing short position.
        --------------------------------------------------------
    */
    {
        const std::vector<TargetPosition> current{
            {"AAPL", -100.0}
        };

        const std::vector<TargetPosition> target{
            {"AAPL", -25.0}
        };

        const auto orders =
            generate_orders(
                current,
                target,
                500
            );

        assert(
            orders.size() == 1
        );

        assert(
            orders[0].side ==
            OrderSide::Buy
        );

        assert(
            orders[0].quantity == 75.0
        );
    }

    /*
        --------------------------------------------------------
        Long -> short reversal.

            current = +100
            target  = -50

            delta = -150

        One sell order is sufficient.
        --------------------------------------------------------
    */
    {
        const std::vector<TargetPosition> current{
            {"AAPL", 100.0}
        };

        const std::vector<TargetPosition> target{
            {"AAPL", -50.0}
        };

        const auto orders =
            generate_orders(
                current,
                target,
                600
            );

        assert(
            orders.size() == 1
        );

        assert(
            orders[0].side ==
            OrderSide::Sell
        );

        assert(
            orders[0].quantity == 150.0
        );
    }

    /*
        --------------------------------------------------------
        Short -> long reversal.

            current = -100
            target  = +50

            delta = +150

        One buy order is sufficient.
        --------------------------------------------------------
    */
    {
        const std::vector<TargetPosition> current{
            {"AAPL", -100.0}
        };

        const std::vector<TargetPosition> target{
            {"AAPL", 50.0}
        };

        const auto orders =
            generate_orders(
                current,
                target,
                700
            );

        assert(
            orders.size() == 1
        );

        assert(
            orders[0].side ==
            OrderSide::Buy
        );

        assert(
            orders[0].quantity == 150.0
        );
    }

    /*
        --------------------------------------------------------
        Exact match -> no order.
        --------------------------------------------------------
    */
    {
        const std::vector<TargetPosition> current{
            {"AAPL", 100.0},
            {"MSFT", -50.0}
        };

        const std::vector<TargetPosition> target{
            {"AAPL", 100.0},
            {"MSFT", -50.0}
        };

        const auto orders =
            generate_orders(
                current,
                target,
                800
            );

        assert(
            orders.empty()
        );
    }

    /*
        --------------------------------------------------------
        Symbol absent from target -> close position.
        --------------------------------------------------------
    */
    {
        const std::vector<TargetPosition> current{
            {"AAPL", 100.0},
            {"MSFT", -50.0}
        };

        const std::vector<TargetPosition> target{
            {"AAPL", 100.0}
        };

        const auto orders =
            generate_orders(
                current,
                target,
                900
            );

        assert(
            orders.size() == 1
        );

        assert(
            orders[0].symbol ==
            "MSFT"
        );

        assert(
            orders[0].side ==
            OrderSide::Buy
        );

        assert(
            orders[0].quantity ==
            50.0
        );
    }

    /*
        --------------------------------------------------------
        Tiny floating-point delta -> no order.
        --------------------------------------------------------
    */
    {
        const std::vector<TargetPosition> current{
            {"AAPL", 100.0}
        };

        const std::vector<TargetPosition> target{
            {"AAPL", 100.0 + 1e-13}
        };

        const auto orders =
            generate_orders(
                current,
                target,
                1000
            );

        assert(
            orders.empty()
        );
    }

    /*
        --------------------------------------------------------
        Duplicate symbols must be rejected.
        --------------------------------------------------------
    */
    {
        const std::vector<TargetPosition> current{
            {"AAPL", 100.0},
            {"AAPL", 200.0}
        };

        const std::vector<TargetPosition> target{};

        bool threw = false;

        try {
            (void)generate_orders(
                current,
                target,
                1100
            );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        --------------------------------------------------------
        Empty symbol must be rejected.
        --------------------------------------------------------
    */
    {
        const std::vector<TargetPosition> current{};

        const std::vector<TargetPosition> target{
            {"", 100.0}
        };

        bool threw = false;

        try {
            (void)generate_orders(
                current,
                target,
                1200
            );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        --------------------------------------------------------
        Non-finite target quantity must be rejected.
        --------------------------------------------------------
    */
    {
        const std::vector<TargetPosition> current{};

        const std::vector<TargetPosition> target{
            {"AAPL", std::nan("1")}
        };

        bool threw = false;

        try {
            (void)generate_orders(
                current,
                target,
                1300
            );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        --------------------------------------------------------
        Negative timestamp must be rejected.
        --------------------------------------------------------
    */
    {
        bool threw = false;

        try {
            (void)generate_orders(
                {},
                {
                    {"AAPL", 100.0}
                },
                -1
            );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        --------------------------------------------------------
        Pair example.

            Current:
                X = +100
                Y = -50

            Target:
                X = +250
                Y = -125

            Required trades:
                Buy 150 X
                Sell 75 Y
        --------------------------------------------------------
    */
    {
        const std::vector<TargetPosition> current{
            {"X", 100.0},
            {"Y", -50.0}
        };

        const std::vector<TargetPosition> target{
            {"X", 250.0},
            {"Y", -125.0}
        };

        const auto orders =
            generate_orders(
                current,
                target,
                1400
            );

        assert(
            orders.size() == 2
        );

        assert(
            orders[0].symbol == "X"
        );

        assert(
            orders[0].side ==
            OrderSide::Buy
        );

        assert(
            orders[0].quantity == 150.0
        );

        assert(
            orders[1].symbol == "Y"
        );

        assert(
            orders[1].side ==
            OrderSide::Sell
        );

        assert(
            orders[1].quantity == 75.0
        );
    }

    std::cout
        << "Order generation tests passed!\n";

    return 0;
}