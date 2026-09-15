#pragma once

#include "quant/execution/order_generation.hpp"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace quant::execution {

struct OperationalRiskLimits {
    double maximum_gross_notional{};

    double maximum_absolute_position{};

    std::size_t maximum_orders_per_cycle{};

    double maximum_daily_loss{};
};

struct RiskViolation {
    std::string symbol;
    std::string message;
};

struct RiskCheckResult {
    bool approved{};
    std::vector<RiskViolation> violations;
};

[[nodiscard]]
RiskCheckResult validate_targets(
    const std::vector<TargetPosition>& current_positions,
    const std::vector<TargetPosition>& target_positions,
    const std::unordered_map<std::string, double>& prices,
    const OperationalRiskLimits& limits,
    double starting_equity,
    double current_equity
);

} // namespace quant::execution