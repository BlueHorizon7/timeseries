#include "quant/execution/execution_engine.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace quant::execution {

OrderId ExecutionEngine::submit(
    Order order,
    std::int64_t timestamp
) {
    if (order.id != 0) {
        throw std::invalid_argument(
            "New orders must have id zero"
        );
    }

    order.id =
        next_order_id_++;

    order.creation_timestamp =
        timestamp;

    order.submission_timestamp =
        0;

    order.last_update_timestamp =
        timestamp;

    order.filled_quantity =
        0.0;

    order.remaining_quantity =
        order.quantity;

    order.average_fill_price =
        0.0;

    order.status =
        OrderStatus::Created;

    validate_order(order);

    submit_order(
        order,
        timestamp
    );

    const OrderId id =
        order.id;

    const auto [iterator, inserted] =
        orders_.emplace(
            id,
            std::move(order)
        );

    if (!inserted) {
        throw std::logic_error(
            "Duplicate generated order id"
        );
    }

    (void)iterator;

    return id;
}

void ExecutionEngine::process_fill(
    const Fill& fill
) {
    validate_fill(fill);

    auto iterator =
        orders_.find(fill.order_id);

    if (iterator == orders_.end()) {
        throw std::out_of_range(
            "Fill references unknown order"
        );
    }

    Order& order =
        iterator->second;

    if (fill.symbol != order.symbol) {
        throw std::invalid_argument(
            "Fill symbol does not match order symbol"
        );
    }

    if (fill.side != order.side) {
        throw std::invalid_argument(
            "Fill side does not match order side"
        );
    }

    if (fill.timestamp <
        order.last_update_timestamp) {

        throw std::invalid_argument(
            "Fill timestamp cannot move backwards"
        );
    }

    apply_fill(
        order,
        fill.quantity,
        fill.price,
        fill.timestamp
    );
}

void ExecutionEngine::cancel(
    OrderId order_id,
    std::int64_t timestamp
) {
    auto iterator =
        orders_.find(order_id);

    if (iterator == orders_.end()) {
        throw std::out_of_range(
            "Unknown order id"
        );
    }

    cancel_order(
        iterator->second,
        timestamp
    );
}

void ExecutionEngine::reject(
    OrderId order_id,
    std::int64_t timestamp
) {
    auto iterator =
        orders_.find(order_id);

    if (iterator == orders_.end()) {
        throw std::out_of_range(
            "Unknown order id"
        );
    }

    reject_order(
        iterator->second,
        timestamp
    );
}

void ExecutionEngine::expire(
    OrderId order_id,
    std::int64_t timestamp
) {
    auto iterator =
        orders_.find(order_id);

    if (iterator == orders_.end()) {
        throw std::out_of_range(
            "Unknown order id"
        );
    }

    expire_order(
        iterator->second,
        timestamp
    );
}

const Order& ExecutionEngine::order(
    OrderId order_id
) const {
    const auto iterator =
        orders_.find(order_id);

    if (iterator == orders_.end()) {
        throw std::out_of_range(
            "Unknown order id"
        );
    }

    return iterator->second;
}

bool ExecutionEngine::contains(
    OrderId order_id
) const {
    return
        orders_.find(order_id) !=
        orders_.end();
}

std::vector<Order>
ExecutionEngine::open_orders() const {
    std::vector<Order> result;

    for (const auto& [id, order] : orders_) {
        (void)id;

        if (is_active(order.status)) {
            result.push_back(order);
        }
    }

    std::sort(
        result.begin(),
        result.end(),
        [](const Order& lhs, const Order& rhs) {
            return lhs.id < rhs.id;
        }
    );

    return result;
}

std::vector<Order>
ExecutionEngine::all_orders() const {
    std::vector<Order> result;

    result.reserve(
        orders_.size()
    );

    for (const auto& [id, order] : orders_) {
        (void)id;
        result.push_back(order);
    }

    std::sort(
        result.begin(),
        result.end(),
        [](const Order& lhs, const Order& rhs) {
            return lhs.id < rhs.id;
        }
    );

    return result;
}

std::size_t ExecutionEngine::size() const {
    return orders_.size();
}

} // namespace quant::execution