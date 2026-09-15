#include "quant/execution/order_generation.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace quant::execution {

namespace {

constexpr double quantity_tolerance = 1e-12;

void validate_target_position(
    const TargetPosition& target
) {
    if (target.symbol.empty()) {
        throw std::invalid_argument(
            "Target position symbol must not be empty"
        );
    }

    if (!std::isfinite(target.quantity)) {
        throw std::invalid_argument(
            "Target position quantity must be finite"
        );
    }
}

std::unordered_map<std::string, double>
to_position_map(
    const std::vector<TargetPosition>& positions
) {
    std::unordered_map<std::string, double> result;

    result.reserve(positions.size());

    for (const auto& position : positions) {
        validate_target_position(position);

        const auto [iterator, inserted] =
            result.emplace(
                position.symbol,
                position.quantity
            );

        if (!inserted) {
            throw std::invalid_argument(
                "Duplicate target/current position symbol: " +
                position.symbol
            );
        }

        (void)iterator;
    }

    return result;
}

} // namespace

std::vector<Order> generate_orders(
    const std::vector<TargetPosition>& current_positions,
    const std::vector<TargetPosition>& target_positions,
    std::int64_t timestamp
) {
    if (timestamp < 0) {
        throw std::invalid_argument(
            "Order generation timestamp must be non-negative"
        );
    }

    const auto current =
        to_position_map(
            current_positions
        );

    const auto target =
        to_position_map(
            target_positions
        );

    /*
        Union of all symbols appearing in either state.
    */
    std::unordered_set<std::string> symbols;

    symbols.reserve(
        current.size() + target.size()
    );

    for (const auto& [symbol, quantity] : current) {
        (void)quantity;
        symbols.insert(symbol);
    }

    for (const auto& [symbol, quantity] : target) {
        (void)quantity;
        symbols.insert(symbol);
    }

    std::vector<std::string> ordered_symbols;

    ordered_symbols.reserve(
        symbols.size()
    );

    for (const auto& symbol : symbols) {
        ordered_symbols.push_back(symbol);
    }

    /*
        Deterministic order generation.
    */
    std::sort(
        ordered_symbols.begin(),
        ordered_symbols.end()
    );

    std::vector<Order> orders;

    orders.reserve(
        ordered_symbols.size()
    );

    for (const auto& symbol : ordered_symbols) {
        const auto current_iterator =
            current.find(symbol);

        const auto target_iterator =
            target.find(symbol);

        const double current_quantity =
            current_iterator == current.end()
                ? 0.0
                : current_iterator->second;

        const double target_quantity =
            target_iterator == target.end()
                ? 0.0
                : target_iterator->second;

        const double delta =
            target_quantity -
            current_quantity;

        if (std::abs(delta) <= quantity_tolerance) {
            continue;
        }

        Order order{};

        order.symbol =
            symbol;

        order.side =
            delta > 0.0
                ? OrderSide::Buy
                : OrderSide::Sell;

        order.type =
            OrderType::Market;

        order.quantity =
            std::abs(delta);

        /*
            A newly generated order receives its real id and
            timestamps when submitted to ExecutionEngine.
        */
        order.id = 0;

        order.status =
            OrderStatus::Created;

        order.filled_quantity =
            0.0;

        order.remaining_quantity =
            order.quantity;

        order.average_fill_price =
            0.0;

        order.creation_timestamp =
            timestamp;

        order.submission_timestamp =
            0;

        order.last_update_timestamp =
            timestamp;

        orders.push_back(
            order
        );
    }

    return orders;
}

} // namespace quant::execution