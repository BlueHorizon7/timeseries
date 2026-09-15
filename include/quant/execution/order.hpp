#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace quant::execution {

using OrderId = std::uint64_t;

enum class OrderSide {
    Buy,
    Sell
};

enum class OrderType {
    Market,
    Limit
};

enum class OrderStatus {
    Created,
    Submitted,
    PartiallyFilled,
    Filled,
    Cancelled,
    Rejected,
    Expired
};

struct Order {
    OrderId id{};

    std::string symbol;

    OrderSide side{};
    OrderType type{};

    double quantity{};
    std::optional<double> limit_price{};

    OrderStatus status{
        OrderStatus::Created
    };

    double filled_quantity{};
    double remaining_quantity{};

    double average_fill_price{};

    std::int64_t creation_timestamp{};
    std::int64_t submission_timestamp{};
    std::int64_t last_update_timestamp{};
};

[[nodiscard]]
bool is_terminal(OrderStatus status);

[[nodiscard]]
bool is_active(OrderStatus status);

[[nodiscard]]
const char* order_side_to_string(
    OrderSide side
);

[[nodiscard]]
const char* order_type_to_string(
    OrderType type
);

[[nodiscard]]
const char* order_status_to_string(
    OrderStatus status
);

void validate_order(
    const Order& order
);

void submit_order(
    Order& order,
    std::int64_t timestamp
);

void cancel_order(
    Order& order,
    std::int64_t timestamp
);

void reject_order(
    Order& order,
    std::int64_t timestamp
);

void expire_order(
    Order& order,
    std::int64_t timestamp
);

void apply_fill(
    Order& order,
    double fill_quantity,
    double fill_price,
    std::int64_t timestamp
);

} // namespace quant::execution