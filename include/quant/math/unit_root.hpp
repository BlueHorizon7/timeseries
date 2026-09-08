#pragma once

namespace quant::math {

enum class UnitRootDecision {
    RejectNull,
    FailToRejectNull
};

struct UnitRootTestResult {
    double statistic{};
    double critical_value_5pct{};
    UnitRootDecision decision{};
};

[[nodiscard]]
UnitRootTestResult
classify_dickey_fuller(
    double statistic
);

} // namespace quant::math