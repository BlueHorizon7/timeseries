#include "quant/research/bootstrap.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>

int main() {
    quant::backtest::BacktestResult backtest;

    constexpr std::size_t number_of_bars = 60;
    constexpr double initial_equity = 100000.0;

    double equity = initial_equity;

    for (std::size_t i = 0;
         i < number_of_bars;
         ++i) {

        const double periodic_return =
            0.0015 *
            std::sin(
                2.0 *
                3.14159265358979323846 *
                static_cast<double>(i) /
                8.0
            );

        const double net_pnl =
            equity * periodic_return;

        equity += net_pnl;

        quant::backtest::BacktestBar bar{};

        bar.timestamp =
            static_cast<std::int64_t>(
                i + 1
            );

        bar.x_price = 100.0;
        bar.y_price = 200.0;

        bar.signal =
            periodic_return > 0.0
                ? 1
                : -1;

        bar.position = {};

        bar.gross_pnl = net_pnl;
        bar.transaction_cost = 0.0;
        bar.execution_cost = 0.0;
        bar.net_pnl = net_pnl;
        bar.equity = equity;

        backtest.bars.push_back(bar);
    }

    quant::research::BootstrapParameters parameters{};

    parameters.number_of_resamples = 500;
    parameters.block_length = 5;
    parameters.random_seed = 123456789ULL;
    parameters.confidence_level = 0.95;
    parameters.periods_per_year = 252.0;

    const auto result =
        quant::research::analyze_block_bootstrap(
            backtest,
            parameters
        );

    assert(
        result.observations ==
        number_of_bars
    );

    assert(
        result.number_of_resamples ==
        500
    );

    assert(
        result.block_length ==
        5
    );

    assert(
        result.total_return_distribution.size() ==
        500
    );

    assert(
        result.annualized_volatility_distribution.size() ==
        500
    );

    assert(
        result.sharpe_ratio_distribution.size() ==
        500
    );

    assert(
        result.maximum_drawdown_distribution.size() ==
        500
    );

    assert(
        result.maximum_drawdown_pct_distribution.size() ==
        500
    );

    assert(
        std::isfinite(
            result.observed_total_return
        )
    );

    assert(
        std::isfinite(
            result.observed_annualized_volatility
        )
    );

    assert(
        std::isfinite(
            result.observed_sharpe_ratio
        )
    );

    assert(
        std::isfinite(
            result.observed_maximum_drawdown
        )
    );

    assert(
        std::isfinite(
            result.observed_maximum_drawdown_pct
        )
    );

    assert(
        result.observed_annualized_volatility >= 0.0
    );

    assert(
        result.observed_maximum_drawdown >= 0.0
    );

    assert(
        result.observed_maximum_drawdown_pct >= 0.0
    );

    assert(
        result.total_return_interval.lower <=
        result.total_return_interval.upper
    );

    assert(
        result.annualized_volatility_interval.lower <=
        result.annualized_volatility_interval.upper
    );

    assert(
        result.sharpe_ratio_interval.lower <=
        result.sharpe_ratio_interval.upper
    );

    assert(
        result.maximum_drawdown_interval.lower <=
        result.maximum_drawdown_interval.upper
    );

    assert(
        result.maximum_drawdown_pct_interval.lower <=
        result.maximum_drawdown_pct_interval.upper
    );

    /*
        Reproducibility: identical seed and inputs must
        produce identical bootstrap distributions.
    */
    const auto repeat_result =
        quant::research::analyze_block_bootstrap(
            backtest,
            parameters
        );

    assert(
        result.total_return_distribution ==
        repeat_result.total_return_distribution
    );

    assert(
        result.sharpe_ratio_distribution ==
        repeat_result.sharpe_ratio_distribution
    );

    /*
        Changing the seed should normally change the
        generated bootstrap path.
    */
    parameters.random_seed =
        987654321ULL;

    const auto different_seed_result =
        quant::research::analyze_block_bootstrap(
            backtest,
            parameters
        );

    assert(
        result.total_return_distribution !=
        different_seed_result.total_return_distribution
    );

    /*
        A block length larger than the number of observations
        is clamped to the sample length rather than rejected.
    */
    parameters.block_length = 10000;

    const auto large_block_result =
        quant::research::analyze_block_bootstrap(
            backtest,
            parameters
        );

    assert(
        large_block_result.block_length ==
        number_of_bars
    );

    /*
        Invalid number of resamples.
    */
    {
        auto invalid = parameters;
        invalid.number_of_resamples = 0;

        bool threw = false;

        try {
            (void)
                quant::research::analyze_block_bootstrap(
                    backtest,
                    invalid
                );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        Invalid block length.
    */
    {
        auto invalid = parameters;
        invalid.block_length = 0;

        bool threw = false;

        try {
            (void)
                quant::research::analyze_block_bootstrap(
                    backtest,
                    invalid
                );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        Invalid confidence level.
    */
    {
        auto invalid = parameters;
        invalid.confidence_level = 1.0;

        bool threw = false;

        try {
            (void)
                quant::research::analyze_block_bootstrap(
                    backtest,
                    invalid
                );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        Invalid annualization frequency.
    */
    {
        auto invalid = parameters;
        invalid.periods_per_year = 0.0;

        bool threw = false;

        try {
            (void)
                quant::research::analyze_block_bootstrap(
                    backtest,
                    invalid
                );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        Empty backtest.
    */
    {
        quant::backtest::BacktestResult empty;

        bool threw = false;

        try {
            (void)
                quant::research::analyze_block_bootstrap(
                    empty,
                    parameters
                );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    std::cout
        << "Bootstrap tests passed!\n";

    return 0;
}