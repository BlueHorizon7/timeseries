#include "quant/execution/simulated_venue.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::execution {

SimulatedExecutionVenue::SimulatedExecutionVenue(
    SimulatedVenueParameters parameters)
    : parameters_(parameters)
{
    if (!std::isfinite(parameters_.buy_slippage_bps) ||
        parameters_.buy_slippage_bps < 0.0) {
        throw std::invalid_argument(
            "buy slippage must be finite and non-negative"
        );
    }

    if (!std::isfinite(parameters_.sell_slippage_bps) ||
        parameters_.sell_slippage_bps < 0.0) {
        throw std::invalid_argument(
            "sell slippage must be finite and non-negative"
        );
    }

    if (!std::isfinite(parameters_.commission_rate) ||
        parameters_.commission_rate < 0.0) {
        throw std::invalid_argument(
            "commission rate must be finite and non-negative"
        );
    }

    if (!std::isfinite(parameters_.partial_fill_fraction) ||
        parameters_.partial_fill_fraction <= 0.0 ||
        parameters_.partial_fill_fraction > 1.0) {
        throw std::invalid_argument(
            "partial fill fraction must be in (0, 1]"
        );
    }

    if (parameters_.latency < 0) {
        throw std::invalid_argument(
            "latency must be non-negative"
        );
    }
}

std::vector<Fill> SimulatedExecutionVenue::execute(
    const Order& order,
    double market_price,
    std::int64_t market_timestamp) const
{
    validate_order(order);

    if (!is_active(order.status)) {
        throw std::invalid_argument(
            "only active orders may be executed"
        );
    }

    if (!std::isfinite(market_price) || market_price <= 0.0) {
        throw std::invalid_argument(
            "market price must be finite and positive"
        );
    }

    if (market_timestamp < 0) {
        throw std::invalid_argument(
            "market timestamp must be non-negative"
        );
    }

    double fill_quantity =
        order.remaining_quantity *
        parameters_.partial_fill_fraction;

    if (fill_quantity <= 0.0) {
        return {};
    }

    if (fill_quantity > order.remaining_quantity) {
        fill_quantity = order.remaining_quantity;
    }

    double slippage_bps = 0.0;

    if (order.side == OrderSide::Buy) {
        slippage_bps = parameters_.buy_slippage_bps;
    } else {
        slippage_bps = parameters_.sell_slippage_bps;
    }

    double fill_price = market_price;

    if (order.side == OrderSide::Buy) {
        fill_price *= 1.0 + slippage_bps / 10000.0;
    } else {
        fill_price *= 1.0 - slippage_bps / 10000.0;
    }

    if (order.type == OrderType::Limit) {
        if (!order.limit_price.has_value()) {
            throw std::logic_error(
                "limit order has no limit price"
            );
        }

        const double limit = *order.limit_price;

        if (order.side == OrderSide::Buy &&
            fill_price > limit) {
            return {};
        }

        if (order.side == OrderSide::Sell &&
            fill_price < limit) {
            return {};
        }
    }

    const double commission =
        fill_quantity *
        fill_price *
        parameters_.commission_rate;

    Fill fill{};
    fill.order_id = order.id;
    fill.symbol = order.symbol;
    fill.side = order.side;
    fill.quantity = fill_quantity;
    fill.price = fill_price;
    fill.timestamp =
        market_timestamp + parameters_.latency;
    fill.commission = commission;

    validate_fill(fill);

    return {fill};
}

} // namespace quant::execution