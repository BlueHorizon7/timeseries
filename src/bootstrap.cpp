#include "quant/research/bootstrap.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace quant::research {

namespace {

struct ReturnStatistics {
    double total_return{};
    double annualized_volatility{};
    double sharpe_ratio{};
    double maximum_drawdown{};
    double maximum_drawdown_pct{};
};

void validate_parameters(
    const BootstrapParameters& parameters
) {
    if (parameters.number_of_resamples == 0) {
        throw std::invalid_argument(
            "Bootstrap requires at least one resample"
        );
    }

    if (parameters.block_length == 0) {
        throw std::invalid_argument(
            "Bootstrap block length must be positive"
        );
    }

    if (parameters.confidence_level <= 0.0 ||
        parameters.confidence_level >= 1.0 ||
        !std::isfinite(parameters.confidence_level)) {

        throw std::invalid_argument(
            "Bootstrap confidence level must be in (0, 1)"
        );
    }

    if (parameters.periods_per_year <= 0.0 ||
        !std::isfinite(parameters.periods_per_year)) {

        throw std::invalid_argument(
            "Bootstrap periods per year must be positive"
        );
    }
}

std::vector<double> extract_returns(
    const quant::backtest::BacktestResult& backtest
) {
    if (backtest.bars.empty()) {
        throw std::invalid_argument(
            "Bootstrap requires at least one backtest bar"
        );
    }

    std::vector<double> returns;
    returns.reserve(backtest.bars.size());

    /*
        The first bar's equity is:

            initial_equity + first_bar.net_pnl

        Therefore:

            initial_equity =
                first_bar.equity - first_bar.net_pnl

        Each subsequent return is the bar PnL divided by the
        equity immediately before that bar.

        Terminal liquidation is deliberately excluded because
        liquidation is represented separately from the
        chronological OOS bar sequence.
    */
    double previous_equity =
        backtest.bars.front().equity -
        backtest.bars.front().net_pnl;

    if (!std::isfinite(previous_equity) ||
        previous_equity <= 0.0) {

        throw std::invalid_argument(
            "Bootstrap requires positive initial equity"
        );
    }

    for (const auto& bar : backtest.bars) {
        if (!std::isfinite(bar.net_pnl) ||
            !std::isfinite(bar.equity)) {

            throw std::invalid_argument(
                "Bootstrap encountered non-finite backtest values"
            );
        }

        if (previous_equity <= 0.0 ||
            !std::isfinite(previous_equity)) {

            throw std::invalid_argument(
                "Bootstrap encountered non-positive equity"
            );
        }

        const double return_value =
            bar.net_pnl / previous_equity;

        if (!std::isfinite(return_value)) {
            throw std::invalid_argument(
                "Bootstrap encountered a non-finite return"
            );
        }

        /*
            A return <= -100% would make the multiplicative
            equity path invalid for this bootstrap model.
        */
        if (return_value <= -1.0) {
            throw std::invalid_argument(
                "Bootstrap encountered a return <= -100%"
            );
        }

        returns.push_back(return_value);

        previous_equity = bar.equity;
    }

    return returns;
}

double mean(
    const std::vector<double>& values
) {
    if (values.empty()) {
        throw std::invalid_argument(
            "Cannot calculate mean of an empty sequence"
        );
    }

    long double total = 0.0L;

    for (const double value : values) {
        total += static_cast<long double>(value);
    }

    return static_cast<double>(
        total /
        static_cast<long double>(values.size())
    );
}

double sample_standard_deviation(
    const std::vector<double>& values
) {
    if (values.size() < 2) {
        return 0.0;
    }

    const double average = mean(values);

    long double squared_sum = 0.0L;

    for (const double value : values) {
        const long double deviation =
            static_cast<long double>(value) -
            static_cast<long double>(average);

        squared_sum += deviation * deviation;
    }

    return static_cast<double>(
        std::sqrt(
            squared_sum /
            static_cast<long double>(values.size() - 1)
        )
    );
}

ReturnStatistics calculate_statistics(
    const std::vector<double>& returns,
    double periods_per_year
) {
    if (returns.empty()) {
        throw std::invalid_argument(
            "Cannot calculate statistics from empty returns"
        );
    }

    double equity = 1.0;
    double peak_equity = equity;

    double maximum_drawdown_pct = 0.0;

    for (const double return_value : returns) {
        equity *= (1.0 + return_value);

        if (!std::isfinite(equity) ||
            equity <= 0.0) {

            throw std::invalid_argument(
                "Bootstrap equity path became invalid"
            );
        }

        peak_equity =
            std::max(peak_equity, equity);

        const double drawdown_pct =
            (peak_equity - equity) /
            peak_equity;

        maximum_drawdown_pct =
            std::max(
                maximum_drawdown_pct,
                drawdown_pct
            );
    }

    const double total_return =
        equity - 1.0;

    const double volatility =
        sample_standard_deviation(returns) *
        std::sqrt(periods_per_year);

    const double average_return =
        mean(returns);

    const double return_std =
        sample_standard_deviation(returns);

    double sharpe_ratio = 0.0;

    if (return_std > 0.0) {
        sharpe_ratio =
            average_return /
            return_std *
            std::sqrt(periods_per_year);
    }

    /*
        The return path is normalized to an initial equity of 1.
        Therefore dollar maximum drawdown is numerically equal
        to percentage maximum drawdown in this normalized path.
    */
    const double maximum_drawdown =
        maximum_drawdown_pct;

    return ReturnStatistics{
        total_return,
        volatility,
        sharpe_ratio,
        maximum_drawdown,
        maximum_drawdown_pct
    };
}

double percentile(
    std::vector<double> values,
    double probability
) {
    if (values.empty()) {
        throw std::invalid_argument(
            "Cannot calculate percentile of empty values"
        );
    }

    if (probability < 0.0 ||
        probability > 1.0) {

        throw std::invalid_argument(
            "Percentile probability must be in [0, 1]"
        );
    }

    std::sort(
        values.begin(),
        values.end()
    );

    if (values.size() == 1) {
        return values.front();
    }

    const double position =
        probability *
        static_cast<double>(values.size() - 1);

    const std::size_t lower =
        static_cast<std::size_t>(
            std::floor(position)
        );

    const std::size_t upper =
        static_cast<std::size_t>(
            std::ceil(position)
        );

    if (lower == upper) {
        return values[lower];
    }

    const double weight =
        position -
        static_cast<double>(lower);

    return
        values[lower] * (1.0 - weight) +
        values[upper] * weight;
}

BootstrapInterval make_interval(
    const std::vector<double>& distribution,
    double confidence_level
) {
    const double alpha =
        (1.0 - confidence_level) / 2.0;

    return BootstrapInterval{
        percentile(distribution, alpha),
        percentile(distribution, 1.0 - alpha)
    };
}

std::vector<double> bootstrap_sample(
    const std::vector<double>& returns,
    std::size_t block_length,
    std::mt19937_64& generator
) {
    const std::size_t n =
        returns.size();

    if (block_length > n) {
        block_length = n;
    }

    std::uniform_int_distribution<std::size_t>
        start_distribution(
            0,
            n - block_length
        );

    std::vector<double> sample;
    sample.reserve(n);

    /*
        Moving-block bootstrap:

        1. Select a block starting point uniformly.
        2. Copy a contiguous block.
        3. Repeat with replacement until the original
           sample length is reconstructed.

        This preserves short-range serial dependence
        within each sampled block.
    */
    while (sample.size() < n) {
        const std::size_t start =
            start_distribution(generator);

        const std::size_t remaining =
            n - sample.size();

        const std::size_t count =
            std::min(
                block_length,
                remaining
            );

        for (std::size_t i = 0;
             i < count;
             ++i) {

            sample.push_back(
                returns[start + i]
            );
        }
    }

    return sample;
}

} // namespace

BootstrapResult analyze_block_bootstrap(
    const quant::backtest::BacktestResult& backtest,
    const BootstrapParameters& parameters
) {
    validate_parameters(parameters);

    const auto returns =
        extract_returns(backtest);

    if (parameters.block_length == 0) {
        throw std::invalid_argument(
            "Bootstrap block length must be positive"
        );
    }

    const ReturnStatistics observed =
        calculate_statistics(
            returns,
            parameters.periods_per_year
        );

    BootstrapResult result{};

    result.observations =
        returns.size();

    result.number_of_resamples =
        parameters.number_of_resamples;

    result.block_length =
        std::min(
            parameters.block_length,
            returns.size()
        );

    result.observed_total_return =
        observed.total_return;

    result.observed_annualized_volatility =
        observed.annualized_volatility;

    result.observed_sharpe_ratio =
        observed.sharpe_ratio;

    result.observed_maximum_drawdown =
        observed.maximum_drawdown;

    result.observed_maximum_drawdown_pct =
        observed.maximum_drawdown_pct;

    result.total_return_distribution.reserve(
        parameters.number_of_resamples
    );

    result.annualized_volatility_distribution.reserve(
        parameters.number_of_resamples
    );

    result.sharpe_ratio_distribution.reserve(
        parameters.number_of_resamples
    );

    result.maximum_drawdown_distribution.reserve(
        parameters.number_of_resamples
    );

    result.maximum_drawdown_pct_distribution.reserve(
        parameters.number_of_resamples
    );

    std::mt19937_64 generator(
        parameters.random_seed
    );

    for (std::size_t iteration = 0;
         iteration < parameters.number_of_resamples;
         ++iteration) {

        const auto sample =
            bootstrap_sample(
                returns,
                result.block_length,
                generator
            );

        const ReturnStatistics statistics =
            calculate_statistics(
                sample,
                parameters.periods_per_year
            );

        result.total_return_distribution.push_back(
            statistics.total_return
        );

        result.annualized_volatility_distribution.push_back(
            statistics.annualized_volatility
        );

        result.sharpe_ratio_distribution.push_back(
            statistics.sharpe_ratio
        );

        result.maximum_drawdown_distribution.push_back(
            statistics.maximum_drawdown
        );

        result.maximum_drawdown_pct_distribution.push_back(
            statistics.maximum_drawdown_pct
        );
    }

    result.total_return_interval =
        make_interval(
            result.total_return_distribution,
            parameters.confidence_level
        );

    result.annualized_volatility_interval =
        make_interval(
            result.annualized_volatility_distribution,
            parameters.confidence_level
        );

    result.sharpe_ratio_interval =
        make_interval(
            result.sharpe_ratio_distribution,
            parameters.confidence_level
        );

    result.maximum_drawdown_interval =
        make_interval(
            result.maximum_drawdown_distribution,
            parameters.confidence_level
        );

    result.maximum_drawdown_pct_interval =
        make_interval(
            result.maximum_drawdown_pct_distribution,
            parameters.confidence_level
        );

    return result;
}

} // namespace quant::research