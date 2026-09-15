#include "quant/execution/order.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>

int main() {
    using namespace quant::execution;

    Order order{};

    order.id = 1;
    order.symbol = "TEST";
    order.side = OrderSide::Buy;
    order.type = OrderType::Market;
    order.quantity = 100.0;

    order.filled_quantity = 0.0;
    order.remaining_quantity = 100.0;

    order.average_fill_price = 0.0;

    order.creation_timestamp = 1;
    order.submission_timestamp = 0;
    order.last_update_timestamp = 1;

    validate_order(order);

    assert(
        order_status_to_string(
            OrderStatus::Created
        ) == std::string("Created")
    );

    assert(
        order_side_to_string(
            OrderSide::Buy
        ) == std::string("Buy")
    );

    assert(
        order_type_to_string(
            OrderType::Market
        ) == std::string("Market")
    );

    assert(
        !is_terminal(
            OrderStatus::Created
        )
    );

    assert(
        !is_active(
            OrderStatus::Created
        )
    );

    submit_order(
        order,
        2
    );

    assert(
        order.status ==
        OrderStatus::Submitted
    );

    assert(
        is_active(order.status)
    );

    apply_fill(
        order,
        40.0,
        100.0,
        3
    );

    assert(
        order.status ==
        OrderStatus::PartiallyFilled
    );

    assert(
        std::abs(
            order.filled_quantity -
            40.0
        ) < 1e-12
    );

    assert(
        std::abs(
            order.remaining_quantity -
            60.0
        ) < 1e-12
    );

    assert(
        std::abs(
            order.average_fill_price -
            100.0
        ) < 1e-12
    );

    apply_fill(
        order,
        60.0,
        102.0,
        4
    );

    assert(
        order.status ==
        OrderStatus::Filled
    );

    assert(
        order.remaining_quantity ==
        0.0
    );

    assert(
        std::abs(
            order.average_fill_price -
            101.2
        ) < 1e-12
    );

    assert(
        is_terminal(order.status)
    );

    {
        bool threw = false;

        try {
            apply_fill(
                order,
                1.0,
                103.0,
                5
            );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    {
        Order cancel_test{};

        cancel_test.id = 2;
        cancel_test.symbol = "TEST";
        cancel_test.side = OrderSide::Sell;
        cancel_test.type = OrderType::Market;
        cancel_test.quantity = 10.0;
        cancel_test.remaining_quantity = 10.0;

        submit_order(
            cancel_test,
            10
        );

        cancel_order(
            cancel_test,
            11
        );

        assert(
            cancel_test.status ==
            OrderStatus::Cancelled
        );

        assert(
            is_terminal(
                cancel_test.status
            )
        );
    }

    {
        Order reject_test{};

        reject_test.id = 3;
        reject_test.symbol = "TEST";
        reject_test.side = OrderSide::Buy;
        reject_test.type = OrderType::Market;
        reject_test.quantity = 10.0;
        reject_test.remaining_quantity = 10.0;

        reject_order(
            reject_test,
            20
        );

        assert(
            reject_test.status ==
            OrderStatus::Rejected
        );
    }

    {
        Order expire_test{};

        expire_test.id = 4;
        expire_test.symbol = "TEST";
        expire_test.side = OrderSide::Buy;
        expire_test.type = OrderType::Market;
        expire_test.quantity = 10.0;
        expire_test.remaining_quantity = 10.0;

        submit_order(
            expire_test,
            30
        );

        expire_order(
            expire_test,
            31
        );

        assert(
            expire_test.status ==
            OrderStatus::Expired
        );
    }

    {
        Order invalid_limit{};

        invalid_limit.id = 5;
        invalid_limit.symbol = "TEST";
        invalid_limit.side = OrderSide::Buy;
        invalid_limit.type = OrderType::Limit;
        invalid_limit.quantity = 10.0;
        invalid_limit.remaining_quantity = 10.0;

        bool threw = false;

        try {
            validate_order(invalid_limit);
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    {
        Order overfill{};

        overfill.id = 6;
        overfill.symbol = "TEST";
        overfill.side = OrderSide::Buy;
        overfill.type = OrderType::Market;
        overfill.quantity = 10.0;
        overfill.remaining_quantity = 10.0;

        submit_order(
            overfill,
            40
        );

        bool threw = false;

        try {
            apply_fill(
                overfill,
                11.0,
                100.0,
                41
            );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    std::cout
        << "Order tests passed!\n";

    return 0;
}