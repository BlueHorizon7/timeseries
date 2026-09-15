#pragma once

#include "quant/execution/order.hpp"

#include <cstdint>
#include <string>

namespace quant::execution {

struct Fill {
    OrderId order_id{};

    std::string symbol;

    OrderSide side{};

    double quantity{};
    double price{};

    std::int64_t timestamp{};

    double commission{};
};

void validate_fill(
    const Fill& fill
);

} // namespace quant::execution