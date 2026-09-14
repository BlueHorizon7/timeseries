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

struct ADFRegressionResult {
    double gamma{};
    double standard_error{};
    double rss{};

    std::size_t observations{};
    std::size_t parameters{};

    std::size_t lags{};
    std::size_t sample_lag{};

    DeterministicTerm deterministic{};
};

[[nodiscard]]
ADFRegressionResult fit_augmented_dickey_fuller(
    const Series& series,
    std::size_t lags,
    DeterministicTerm deterministic,
    std::size_t sample_lag
);

[[nodiscard]]
ADFResult augmented_dickey_fuller(
    const Series& series,
    std::size_t lags,
    DeterministicTerm deterministic
);

[[nodiscard]]
ADFResult augmented_dickey_fuller(
    const Series& series,
    std::size_t lags,
    DeterministicTerm deterministic,
    std::size_t sample_lag
);

} // namespace quant::math