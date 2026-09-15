#include "quant/execution/position.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::execution {

namespace {

void validate_fill_inputs(
    double quantity,
    double price,
    double commission
) {
    if (!std::isfinite(quantity) ||
        quantity <= 0.0) {
        throw std::invalid_argument(
            "Fill quantity must be positive"
        );
    }

    if (!std::isfinite(price) ||
        price <= 0.0) {
        throw std::invalid_argument(
            "Fill price must be positive"
        );
    }

    if (!std::isfinite(commission) ||
        commission < 0.0) {
        throw std::invalid_argument(
            "Commission must be non-negative"
        );
    }
}

} // namespace

void apply_position_fill(
    Position& position,
    bool is_buy,
    double fill_quantity,
    double fill_price,
    double commission,
    std::int64_t timestamp
) {
    validate_fill_inputs(
        fill_quantity,
        fill_price,
        commission
    );

    if (position.symbol.empty()) {
        throw std::invalid_argument(
            "Position symbol must not be empty"
        );
    }

    if (timestamp < position.last_update_timestamp) {
        throw std::invalid_argument(
            "Position timestamp cannot move backwards"
        );
    }

    const double old_quantity =
        position.quantity;

    const double signed_quantity =
        is_buy
            ? fill_quantity
            : -fill_quantity;

    /*
        Opening or increasing an existing position.
    */
    if (old_quantity == 0.0 ||
        (old_quantity > 0.0 &&
         signed_quantity > 0.0) ||
        (old_quantity < 0.0 &&
         signed_quantity < 0.0)) {

        const double old_abs =
            std::abs(old_quantity);

        const double new_abs =
            old_abs + fill_quantity;

        position.average_entry_price =
            old_abs == 0.0
                ? fill_price
                : (
                    position.average_entry_price *
                        old_abs +
                    fill_price *
                        fill_quantity
                ) /
                    new_abs;

        position.quantity =
            old_quantity + signed_quantity;
    } else {
        /*
            Closing or reversing a position.
        */
        const double closing_quantity =
            std::min(
                std::abs(old_quantity),
                fill_quantity
            );

        const double pnl_per_unit =
            old_quantity > 0.0
                ? fill_price -
                      position.average_entry_price
                : position.average_entry_price -
                      fill_price;

        position.realized_pnl +=
            closing_quantity *
            pnl_per_unit;

        const double remaining_fill =
            fill_quantity -
            closing_quantity;

        position.quantity =
            old_quantity +
            signed_quantity;

        if (position.quantity == 0.0) {
            position.average_entry_price =
                0.0;
        } else if (remaining_fill > 0.0) {
            position.average_entry_price =
                fill_price;
        }
    }

    position.total_commission +=
        commission;

    position.realized_pnl -=
        commission;

    position.unrealized_pnl =
        0.0;

    position.last_update_timestamp =
        timestamp;
}

void mark_position(
    Position& position,
    double market_price,
    std::int64_t timestamp
) {
    if (!std::isfinite(market_price) ||
        market_price <= 0.0) {
        throw std::invalid_argument(
            "Market price must be positive"
        );
    }

    if (timestamp < position.last_update_timestamp) {
        throw std::invalid_argument(
            "Position timestamp cannot move backwards"
        );
    }

    if (position.quantity == 0.0) {
        position.unrealized_pnl = 0.0;
    } else if (position.quantity > 0.0) {
        position.unrealized_pnl =
            position.quantity *
            (
                market_price -
                position.average_entry_price
            );
    } else {
        position.unrealized_pnl =
            std::abs(position.quantity) *
            (
                position.average_entry_price -
                market_price
            );
    }

    position.last_update_timestamp =
        timestamp;
}

} // namespace quant::execution