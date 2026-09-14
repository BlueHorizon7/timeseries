#pragma once

#include "quant/data/aligned_series.hpp"
#include "quant/math/cointegration.hpp"
#include "quant/research/pairs_research.hpp"
#include "quant/risk/performance.hpp"
#include "quant/backtest/backtest.hpp"

#include <cstddef>
#include <optional>
#include <vector>

namespace quant::research {

struct WalkForwardParameters {
    std::size_t formation_size{};
    std::size_t test_size{};

    /*
        Cointegration/ADF configuration used independently
        inside every formation fold.
    */
    quant::math::CointegrationParameters
        cointegration{};

    PairsResearchParameters strategy{};
};

struct WalkForwardFold {
    std::size_t formation_begin{};
    std::size_t formation_end{};

    std::size_t test_begin{};
    std::size_t test_end{};

    bool formation_passed{};

    std::optional<
        quant::risk::PerformanceMetrics
    > performance;
};

struct WalkForwardResult {
    std::vector<WalkForwardFold> folds;

    /*
        The single authoritative chronologically
        combined OOS timeline.
    */
    std::optional<
        quant::backtest::BacktestResult
    > aggregate_backtest;

    std::optional<
        quant::risk::PerformanceMetrics
    > aggregate_performance;
};

[[nodiscard]]
WalkForwardResult run_walk_forward(
    const quant::data::AlignedSeries& data,
    const WalkForwardParameters& parameters,
    double periods_per_year
);

} // namespace quant::research