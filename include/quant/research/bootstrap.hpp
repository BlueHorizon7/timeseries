#pragma once

#include "quant/backtest/backtest.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace quant::research {

struct BootstrapParameters {
    std::size_t number_of_resamples{1000};
    std::size_t block_length{5};
    std::uint64_t random_seed{123456789ULL};
    double confidence_level{0.95};
    double periods_per_year{252.0};
};

struct BootstrapInterval {
    double lower{};
    double upper{};
};

struct BootstrapResult {
    std::size_t observations{};
    std::size_t number_of_resamples{};
    std::size_t block_length{};

    double observed_total_return{};
    double observed_annualized_volatility{};
    double observed_sharpe_ratio{};
    double observed_maximum_drawdown{};
    double observed_maximum_drawdown_pct{};

    std::vector<double> total_return_distribution;
    std::vector<double> annualized_volatility_distribution;
    std::vector<double> sharpe_ratio_distribution;
    std::vector<double> maximum_drawdown_distribution;
    std::vector<double> maximum_drawdown_pct_distribution;

    BootstrapInterval total_return_interval{};
    BootstrapInterval annualized_volatility_interval{};
    BootstrapInterval sharpe_ratio_interval{};
    BootstrapInterval maximum_drawdown_interval{};
    BootstrapInterval maximum_drawdown_pct_interval{};
};

[[nodiscard]]
BootstrapResult analyze_block_bootstrap(
    const quant::backtest::BacktestResult& backtest,
    const BootstrapParameters& parameters
);

} // namespace quant::research