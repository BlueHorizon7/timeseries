#include "quant/execution/execution_engine.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>

int main() {
    using namespace quant::execution;

    ExecutionEngine engine;

    Order order{};

    order.symbol = "AAPL";
    order.side = OrderSide::Buy;
    order.type = OrderType::Limit;
    order.quantity = 100.0;
    order.limit_price = 200.0;

    const OrderId id =
        engine.submit(
            order,
            100
        );

    assert(id != 0);

    assert(
        engine.size() == 1
    );

    assert(
        engine.contains(id)
    );

    const auto& submitted =
        engine.order(id);

    assert(
        submitted.id == id
    );

    assert(
        submitted.status ==
        OrderStatus::Submitted
    );

    assert(
        submitted.quantity ==
        100.0
    );

    assert(
        submitted.remaining_quantity ==
        100.0
    );

    assert(
        engine.open_orders().size() == 1
    );

    /*
        First partial fill.
    */
    engine.process_fill(
        Fill{
            id,
            "AAPL",
            OrderSide::Buy,
            25.0,
            199.0,
            101,
            0.10
        }
    );

    {
        const auto& current =
            engine.order(id);

        assert(
            current.status ==
            OrderStatus::PartiallyFilled
        );

        assert(
            std::abs(
                current.filled_quantity -
                25.0
            ) < 1e-12
        );

        assert(
            std::abs(
                current.remaining_quantity -
                75.0
            ) < 1e-12
        );

        assert(
            std::abs(
                current.average_fill_price -
                199.0
            ) < 1e-12
        );
    }

    /*
        Second partial fill.
    */
    engine.process_fill(
        Fill{
            id,
            "AAPL",
            OrderSide::Buy,
            25.0,
            201.0,
            102,
            0.10
        }
    );

    {
        const auto& current =
            engine.order(id);

        assert(
            current.status ==
            OrderStatus::PartiallyFilled
        );

        assert(
            std::abs(
                current.filled_quantity -
                50.0
            ) < 1e-12
        );

        assert(
            std::abs(
                current.remaining_quantity -
                50.0
            ) < 1e-12
        );

        assert(
            std::abs(
                current.average_fill_price -
                200.0
            ) < 1e-12
        );
    }

    /*
        Final fill.
    */
    engine.process_fill(
        Fill{
            id,
            "AAPL",
            OrderSide::Buy,
            50.0,
            202.0,
            103,
            0.20
        }
    );

    {
        const auto& current =
            engine.order(id);

        assert(
            current.status ==
            OrderStatus::Filled
        );

        assert(
            current.filled_quantity ==
            100.0
        );

        assert(
            current.remaining_quantity ==
            0.0
        );

        assert(
            std::abs(
                current.average_fill_price -
                201.0
            ) < 1e-12
        );
    }

    assert(
        engine.open_orders().empty()
    );

    /*
        A filled order may not receive another fill.
    */
    {
        bool threw = false;

        try {
            engine.process_fill(
                Fill{
                    id,
                    "AAPL",
                    OrderSide::Buy,
                    1.0,
                    205.0,
                    104,
                    0.01
                }
            );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        Unknown order.
    */
    {
        bool threw = false;

        try {
            (void)engine.order(9999);
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        Cancellation.
    */
    {
        Order cancel_order{};

        cancel_order.symbol = "MSFT";
        cancel_order.side = OrderSide::Sell;
        cancel_order.type = OrderType::Market;
        cancel_order.quantity = 50.0;

        const OrderId cancel_id =
            engine.submit(
                cancel_order,
                200
            );

        engine.cancel(
            cancel_id,
            201
        );

        assert(
            engine.order(cancel_id).status ==
            OrderStatus::Cancelled
        );

        assert(
            engine.open_orders().size() == 0
        );
    }

    /*
        Rejection.
    */
    {
        Order reject_order{};

        reject_order.symbol = "GOOG";
        reject_order.side = OrderSide::Buy;
        reject_order.type = OrderType::Market;
        reject_order.quantity = 10.0;

        const OrderId reject_id =
            engine.submit(
                reject_order,
                300
            );

        engine.reject(
            reject_id,
            301
        );

        assert(
            engine.order(reject_id).status ==
            OrderStatus::Rejected
        );
    }

    /*
        Expiration.
    */
    {
        Order expire_order{};

        expire_order.symbol = "TSLA";
        expire_order.side = OrderSide::Buy;
        expire_order.type = OrderType::Market;
        expire_order.quantity = 10.0;

        const OrderId expire_id =
            engine.submit(
                expire_order,
                400
            );

        engine.expire(
            expire_id,
            401
        );

        assert(
            engine.order(expire_id).status ==
            OrderStatus::Expired
        );
    }

    /*
        Fill/order mismatch must be rejected.
    */
    {
        Order mismatch{};

        mismatch.symbol = "AMZN";
        mismatch.side = OrderSide::Buy;
        mismatch.type = OrderType::Market;
        mismatch.quantity = 10.0;

        const OrderId mismatch_id =
            engine.submit(
                mismatch,
                500
            );

        bool threw = false;

        try {
            engine.process_fill(
                Fill{
                    mismatch_id,
                    "MSFT",
                    OrderSide::Buy,
                    10.0,
                    100.0,
                    501,
                    0.0
                }
            );
        } catch (...) {
            threw = true;
        }

        assert(threw);

        assert(
            engine.order(mismatch_id).status ==
            OrderStatus::Submitted
        );
    }

    /*
        Fill side mismatch must be rejected.
    */
    {
        Order mismatch{};

        mismatch.symbol = "META";
        mismatch.side = OrderSide::Buy;
        mismatch.type = OrderType::Market;
        mismatch.quantity = 10.0;

        const OrderId mismatch_id =
            engine.submit(
                mismatch,
                600
            );

        bool threw = false;

        try {
            engine.process_fill(
                Fill{
                    mismatch_id,
                    "META",
                    OrderSide::Sell,
                    10.0,
                    100.0,
                    601,
                    0.0
                }
            );
        } catch (...) {
            threw = true;
        }

        assert(threw);

        assert(
            engine.order(mismatch_id).status ==
            OrderStatus::Submitted
        );
    }

    /*
        Historical fill timestamps must be rejected.
    */
    {
        Order timestamp_test{};

        timestamp_test.symbol = "NFLX";
        timestamp_test.side = OrderSide::Buy;
        timestamp_test.type = OrderType::Market;
        timestamp_test.quantity = 10.0;

        const OrderId timestamp_id =
            engine.submit(
                timestamp_test,
                700
            );

        bool threw = false;

        try {
            engine.process_fill(
                Fill{
                    timestamp_id,
                    "NFLX",
                    OrderSide::Buy,
                    10.0,
                    100.0,
                    699,
                    0.0
                }
            );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        Orders must be returned deterministically by id.
    */
    {
        const auto all =
            engine.all_orders();

        assert(
            all.size() ==
            engine.size()
        );

        for (std::size_t i = 1;
             i < all.size();
             ++i) {

            assert(
                all[i - 1].id <
                all[i].id
            );
        }
    }

    std::cout
        << "Execution engine tests passed!\n";

    return 0;
}