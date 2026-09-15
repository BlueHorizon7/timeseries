#include "quant/execution/simulated_venue.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

int main()
{
    using namespace quant::execution;

    SimulatedExecutionVenue venue({
        .buy_slippage_bps = 10.0,
        .sell_slippage_bps = 10.0,
        .commission_rate = 0.001,
        .partial_fill_fraction = 0.5,
        .latency = 2
    });

    Order order{};
    order.id = 1;
    order.symbol = "XYZ";
    order.side = OrderSide::Buy;
    order.type = OrderType::Market;
    order.quantity = 100.0;
    order.remaining_quantity = 100.0;
    order.status = OrderStatus::Submitted;

    const auto fills =
        venue.execute(order, 100.0, 1000);

    assert(fills.size() == 1);

    const Fill& fill = fills.front();

    assert(fill.quantity == 50.0);
    assert(std::abs(fill.price - 100.10) < 1e-10);
    assert(fill.timestamp == 1002);
    assert(
        std::abs(fill.commission - 5.005) < 1e-10
    );

    std::cout << "simulated_venue_test passed\n";
}