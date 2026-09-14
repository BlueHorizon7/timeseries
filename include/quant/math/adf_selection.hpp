#pragma once

#include "quant/math/adf.hpp"

#include <cstddef>

namespace quant::math {

enum class InformationCriterion {
    AIC,
    BIC
};

struct ADFLagSelectionResult {
    std::size_t selected_lags{};
    double criterion_value{};
    ADFResult adf{};
};

[[nodiscard]]
ADFLagSelectionResult select_adf_lag(
    const Series& series,
    std::size_t max_lags,
    DeterministicTerm deterministic,
    InformationCriterion criterion
);

} // namespace quant::math