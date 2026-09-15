#include "quant/execution/risk_controls.hpp"

#include <cmath>
#include <unordered_map>

namespace quant::execution {

RiskCheckResult validate_targets(
    const std::vector<TargetPosition>& current_positions,
    const std::vector<TargetPosition>& target_positions,
    const std::unordered_map<std::string, double>& prices,
    const OperationalRiskLimits& limits,
    double starting_equity,
    double current_equity)
{
    RiskCheckResult result{};
    result.approved = true;

    if (!std::isfinite(starting_equity) ||
        !std::isfinite(current_equity)) {

        result.approved = false;
        result.violations.push_back({
            "",
            "equity must be finite"
        });

        return result;
    }

    const double loss =
        starting_equity - current_equity;

    if (loss > limits.maximum_daily_loss) {
        result.approved = false;
        result.violations.push_back({
            "",
            "maximum daily loss exceeded"
        });
    }

    if (target_positions.size() >
        limits.maximum_orders_per_cycle) {

        result.approved = false;
        result.violations.push_back({
            "",
            "target count exceeds operational limit"
        });
    }

    std::unordered_map<std::string, double> targets;

    for (const auto& target : target_positions) {
        if (!std::isfinite(target.quantity)) {
            result.approved = false;
            result.violations.push_back({
                target.symbol,
                "target quantity is not finite"
            });
            continue;
        }

        const auto price_it =
            prices.find(target.symbol);

        if (price_it == prices.end()) {
            result.approved = false;
            result.violations.push_back({
                target.symbol,
                "missing price"
            });
            continue;
        }

        if (std::abs(target.quantity) >
            limits.maximum_absolute_position) {

            result.approved = false;
            result.violations.push_back({
                target.symbol,
                "maximum position exceeded"
            });
        }

        targets[target.symbol] += target.quantity;
    }

    double gross_notional = 0.0;

    for (const auto& [symbol, quantity] : targets) {
        const auto price_it = prices.find(symbol);

        if (price_it == prices.end()) {
            continue;
        }

        gross_notional +=
            std::abs(quantity) *
            price_it->second;
    }

    if (gross_notional >
        limits.maximum_gross_notional) {

        result.approved = false;
        result.violations.push_back({
            "",
            "maximum gross notional exceeded"
        });
    }

    static_cast<void>(current_positions);

    return result;
}

} // namespace quant::execution