#include "quant/execution/order.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::execution {

namespace {

void validate_timestamp(
    std::int64_t timestamp
) {
    if (timestamp < 0) {
        throw std::invalid_argument(
            "Order timestamp must be non-negative"
        );
    }
}

void validate_positive_finite(
    double value,
    const char* name
) {
    if (!std::isfinite(value) ||
        value <= 0.0) {

        throw std::invalid_argument(
            std::string(name) +
            " must be finite and positive"
        );
    }
}

} // namespace

bool is_terminal(
    OrderStatus status
) {
    return
        status == OrderStatus::Filled ||
        status == OrderStatus::Cancelled ||
        status == OrderStatus::Rejected ||
        status == OrderStatus::Expired;
}

bool is_active(
    OrderStatus status
) {
    return
        status == OrderStatus::Submitted ||
        status == OrderStatus::PartiallyFilled;
}

const char* order_side_to_string(
    OrderSide side
) {
    switch (side) {
    case OrderSide::Buy:
        return "Buy";

    case OrderSide::Sell:
        return "Sell";
    }

    throw std::invalid_argument(
        "Unknown order side"
    );
}

const char* order_type_to_string(
    OrderType type
) {
    switch (type) {
    case OrderType::Market:
        return "Market";

    case OrderType::Limit:
        return "Limit";
    }

    throw std::invalid_argument(
        "Unknown order type"
    );
}

const char* order_status_to_string(
    OrderStatus status
) {
    switch (status) {
    case OrderStatus::Created:
        return "Created";

    case OrderStatus::Submitted:
        return "Submitted";

    case OrderStatus::PartiallyFilled:
        return "PartiallyFilled";

    case OrderStatus::Filled:
        return "Filled";

    case OrderStatus::Cancelled:
        return "Cancelled";

    case OrderStatus::Rejected:
        return "Rejected";

    case OrderStatus::Expired:
        return "Expired";
    }

    throw std::invalid_argument(
        "Unknown order status"
    );
}

void validate_order(
    const Order& order
) {
    if (order.id == 0) {
        throw std::invalid_argument(
            "Order id must be non-zero"
        );
    }

    if (order.symbol.empty()) {
        throw std::invalid_argument(
            "Order symbol must not be empty"
        );
    }

    validate_positive_finite(
        order.quantity,
        "Order quantity"
    );

    if (order.type == OrderType::Limit) {
        if (!order.limit_price.has_value()) {
            throw std::invalid_argument(
                "Limit order requires a limit price"
            );
        }

        validate_positive_finite(
            *order.limit_price,
            "Limit price"
        );
    } else if (order.limit_price.has_value()) {
        throw std::invalid_argument(
            "Market order must not have a limit price"
        );
    }

    if (!std::isfinite(order.filled_quantity) ||
        order.filled_quantity < 0.0 ||
        order.filled_quantity > order.quantity) {

        throw std::invalid_argument(
            "Invalid filled quantity"
        );
    }

    if (!std::isfinite(order.remaining_quantity) ||
        order.remaining_quantity < 0.0 ||
        order.remaining_quantity > order.quantity) {

        throw std::invalid_argument(
            "Invalid remaining quantity"
        );
    }

    const double quantity_tolerance = 1e-12;

    if (std::abs(
            (order.filled_quantity +
             order.remaining_quantity) -
            order.quantity
        ) > quantity_tolerance) {

        throw std::invalid_argument(
            "Filled quantity plus remaining quantity "
            "must equal order quantity"
        );
    }

    if (!std::isfinite(order.average_fill_price) ||
        order.average_fill_price < 0.0) {

        throw std::invalid_argument(
            "Invalid average fill price"
        );
    }

    validate_timestamp(
        order.creation_timestamp
    );

    validate_timestamp(
        order.submission_timestamp
    );

    validate_timestamp(
        order.last_update_timestamp
    );
}

void submit_order(
    Order& order,
    std::int64_t timestamp
) {
    validate_timestamp(timestamp);

    if (order.status != OrderStatus::Created) {
        throw std::logic_error(
            "Only Created orders may be submitted"
        );
    }

    if (order.id == 0) {
        throw std::invalid_argument(
            "Order id must be non-zero"
        );
    }

    if (order.quantity <= 0.0 ||
        !std::isfinite(order.quantity)) {

        throw std::invalid_argument(
            "Order quantity must be finite and positive"
        );
    }

    order.status =
        OrderStatus::Submitted;

    order.submission_timestamp =
        timestamp;

    order.last_update_timestamp =
        timestamp;
}

void cancel_order(
    Order& order,
    std::int64_t timestamp
) {
    validate_timestamp(timestamp);

    if (!is_active(order.status)) {
        throw std::logic_error(
            "Only active orders may be cancelled"
        );
    }

    order.status =
        OrderStatus::Cancelled;

    order.last_update_timestamp =
        timestamp;
}

void reject_order(
    Order& order,
    std::int64_t timestamp
) {
    validate_timestamp(timestamp);

    if (order.status != OrderStatus::Created &&
        order.status != OrderStatus::Submitted) {

        throw std::logic_error(
            "Order cannot be rejected in its current state"
        );
    }

    order.status =
        OrderStatus::Rejected;

    order.last_update_timestamp =
        timestamp;
}

void expire_order(
    Order& order,
    std::int64_t timestamp
) {
    validate_timestamp(timestamp);

    if (!is_active(order.status)) {
        throw std::logic_error(
            "Only active orders may expire"
        );
    }

    order.status =
        OrderStatus::Expired;

    order.last_update_timestamp =
        timestamp;
}

void apply_fill(
    Order& order,
    double fill_quantity,
    double fill_price,
    std::int64_t timestamp
) {
    validate_timestamp(timestamp);

    validate_positive_finite(
        fill_quantity,
        "Fill quantity"
    );

    validate_positive_finite(
        fill_price,
        "Fill price"
    );

    if (!is_active(order.status)) {
        throw std::logic_error(
            "Only active orders may receive fills"
        );
    }

    const double tolerance = 1e-12;

    if (fill_quantity >
        order.remaining_quantity + tolerance) {

        throw std::invalid_argument(
            "Fill quantity exceeds remaining order quantity"
        );
    }

    const double old_filled =
        order.filled_quantity;

    const double new_filled =
        old_filled +
        fill_quantity;

    const double old_average =
        order.average_fill_price;

    const double weighted_value =
        old_average * old_filled +
        fill_price * fill_quantity;

    order.filled_quantity =
        new_filled;

    order.remaining_quantity =
        order.quantity -
        new_filled;

    if (new_filled > 0.0) {
        order.average_fill_price =
            weighted_value /
            new_filled;
    }

    if (order.remaining_quantity <= tolerance) {
        order.remaining_quantity = 0.0;

        order.status =
            OrderStatus::Filled;
    } else {
        order.status =
            OrderStatus::PartiallyFilled;
    }

    order.last_update_timestamp =
        timestamp;
}

} // namespace quant::execution