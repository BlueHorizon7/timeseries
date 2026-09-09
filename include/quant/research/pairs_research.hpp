#pragma once

#include "quant/backtest/backtest.hpp"
#include "quant/data/aligned_series.hpp"
#include "quant/strategy/pairs_signal.hpp"

#include <cstddef>
#include <vector>

namespace quant::research {

struct PairsResearchParameters {
    std::size_t hedge_ratio_window{};
    std::size_t zscore_window{};

    quant::strategy::SignalParameters signal_parameters{};

    quant::backtest::BacktestParameters backtest_parameters{};
};

struct PairsResearchResult {
    std::vector<std::int64_t> timestamps;

    std::vector<double> x_prices;
    std::vector<double> y_prices;

    std::vector<double> hedge_ratios;
    std::vector<double> spreads;
    std::vector<double> zscores;

    std::vector<int> signals;

    quant::backtest::BacktestResult backtest;
};

[[nodiscard]]
PairsResearchResult
run_pairs_research(
    const quant::data::AlignedSeries& prices,
    const PairsResearchParameters& parameters
);

} // namespace quant::research