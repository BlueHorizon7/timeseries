#pragma once

#include "quant/execution/order.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace quant::execution {

struct TargetPosition {
    std::string symbol;
    double quantity{};
};

[[nodiscard]]
std::vector<Order> generate_orders(
    const std::vector<TargetPosition>& current_positions,
    const std::vector<TargetPosition>& target_positions,
    std::int64_t timestamp
);

} // namespace quant::execution