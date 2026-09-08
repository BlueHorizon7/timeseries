#pragma once

#include "quant/math/adf.hpp"

namespace quant::math {

struct ADFCriticalValues {
    double one_percent{};
    double five_percent{};
    double ten_percent{};
};

enum class ADFDecision {
    RejectAtOnePercent,
    RejectAtFivePercent,
    RejectAtTenPercent,
    FailToReject
};

struct ADFInferenceResult {
    double statistic{};
    ADFCriticalValues critical_values{};
    ADFDecision decision{};
};

[[nodiscard]]
ADFCriticalValues
adf_critical_values(
    DeterministicTerm deterministic
);

[[nodiscard]]
ADFInferenceResult
infer_adf(
    const ADFResult& result
);

} // namespace quant::math