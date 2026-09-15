#pragma once

#include "quant/execution/fill.hpp"
#include "quant/execution/order.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace quant::execution {

class ExecutionEngine {
public:
    ExecutionEngine() = default;

    [[nodiscard]]
    OrderId submit(
        Order order,
        std::int64_t timestamp
    );

    void process_fill(
        const Fill& fill
    );

    void cancel(
        OrderId order_id,
        std::int64_t timestamp
    );

    void reject(
        OrderId order_id,
        std::int64_t timestamp
    );

    void expire(
        OrderId order_id,
        std::int64_t timestamp
    );

    [[nodiscard]]
    const Order& order(
        OrderId order_id
    ) const;

    [[nodiscard]]
    bool contains(
        OrderId order_id
    ) const;

    [[nodiscard]]
    std::vector<Order> open_orders() const;

    [[nodiscard]]
    std::vector<Order> all_orders() const;

    [[nodiscard]]
    std::size_t size() const;

private:
    std::unordered_map<OrderId, Order> orders_;
    OrderId next_order_id_{1};
};

} // namespace quant::execution