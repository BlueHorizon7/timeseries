#include "quant/math/unit_root.hpp"

namespace quant::math {

UnitRootTestResult classify_dickey_fuller(
    double statistic
) {
    // Asymptotic 5% critical value for the
    // Dickey-Fuller test with an intercept.
    constexpr double critical_value_5pct = -2.86;

    const UnitRootDecision decision =
        statistic < critical_value_5pct
            ? UnitRootDecision::RejectNull
            : UnitRootDecision::FailToRejectNull;

    return UnitRootTestResult{
        statistic,
        critical_value_5pct,
        decision
    };
}

} // namespace quant::math