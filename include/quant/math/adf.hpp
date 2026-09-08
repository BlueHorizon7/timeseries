#pragma once

#include "quant/math/series.hpp"

#include <cstddef>

namespace quant::math {

enum class DeterministicTerm {
    None,
    Intercept,
    InterceptAndTrend
};

struct ADFResult {
    double statistic{};
    double gamma{};
    double standard_error{};
    std::size_t lags{};
    DeterministicTerm deterministic{};
};

[[nodiscard]]
ADFResult augmented_dickey_fuller(
    const Series& series,
    std::size_t lags,
    DeterministicTerm deterministic
);

} // namespace quant::math