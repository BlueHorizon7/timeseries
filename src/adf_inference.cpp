#include "quant/math/adf_inference.hpp"

#include <stdexcept>

namespace quant::math {

ADFCriticalValues adf_critical_values(
    DeterministicTerm deterministic
) {
    switch (deterministic) {
        case DeterministicTerm::None:
            return ADFCriticalValues{
                -2.58,
                -1.95,
                -1.62
            };

        case DeterministicTerm::Intercept:
            return ADFCriticalValues{
                -3.43,
                -2.86,
                -2.57
            };

        case DeterministicTerm::InterceptAndTrend:
            return ADFCriticalValues{
                -3.96,
                -3.41,
                -3.13
            };
    }

    throw std::invalid_argument(
        "Unknown deterministic specification"
    );
}

ADFInferenceResult infer_adf(
    const ADFResult& result
) {
    const ADFCriticalValues critical =
        adf_critical_values(result.deterministic);

    ADFDecision decision =
        ADFDecision::FailToReject;

    if (result.statistic < critical.one_percent) {
        decision =
            ADFDecision::RejectAtOnePercent;
    }
    else if (result.statistic < critical.five_percent) {
        decision =
            ADFDecision::RejectAtFivePercent;
    }
    else if (result.statistic < critical.ten_percent) {
        decision =
            ADFDecision::RejectAtTenPercent;
    }

    return ADFInferenceResult{
        result.statistic,
        critical,
        decision
    };
}

} // namespace quant::math