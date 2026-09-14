#include "quant/research/walk_forward.hpp"

#include "quant/math/cointegration.hpp"
#include "quant/math/observation.hpp"
#include "quant/math/series.hpp"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace quant::research {

namespace {

quant::math::Series make_x_series(
    const quant::data::AlignedSeries& data,
    std::size_t begin,
    std::size_t end
) {
    quant::math::Series result;
    result.reserve(end - begin);
    for (std::size_t i = begin; i < end; ++i) {
        result.add(quant::math::Observation{data[i].timestamp, data[i].x});
    }
    return result;
}

quant::math::Series make_y_series(
    const quant::data::AlignedSeries& data,
    std::size_t begin,
    std::size_t end
) {
    quant::math::Series result;
    result.reserve(end - begin);
    for (std::size_t i = begin; i < end; ++i) {
        result.add(quant::math::Observation{data[i].timestamp, data[i].y});
    }
    return result;
}

quant::data::AlignedSeries slice(
    const quant::data::AlignedSeries& data,
    std::size_t begin,
    std::size_t end
) {
    if (begin > end || end > data.size()) {
        throw std::invalid_argument("Invalid aligned-series slice");
    }
    quant::data::AlignedSeries result;
    result.reserve(end - begin);
    for (std::size_t i = begin; i < end; ++i) {
        result.add(data[i]);
    }
    return result;
}

} // namespace

WalkForwardResult run_walk_forward(
    const quant::data::AlignedSeries& data,
    const WalkForwardParameters& parameters,
    double periods_per_year
) {
    if (data.size() < 20) {
        throw std::invalid_argument("Walk-forward validation requires at least 20 observations");
    }
    if (parameters.formation_size < 20) {
        throw std::invalid_argument("Walk-forward formation size must be at least 20");
    }
    if (parameters.test_size < 2) {
        throw std::invalid_argument("Walk-forward test size must be at least two");
    }
    if (parameters.formation_size >= data.size()) {
        throw std::invalid_argument("Formation size must leave observations for testing");
    }
    if (periods_per_year <= 0.0) {
        throw std::invalid_argument("Periods per year must be positive");
    }
    if (parameters.strategy.hedge_ratio_window < 2) {
        throw std::invalid_argument("Hedge ratio window must be at least two");
    }
    if (parameters.strategy.zscore_window < 2) {
        throw std::invalid_argument("Z-score window must be at least two");
    }
    if (parameters.formation_size <= parameters.strategy.hedge_ratio_window) {
        throw std::invalid_argument("Formation size must exceed hedge ratio window");
    }
    if (parameters.formation_size <= parameters.strategy.zscore_window) {
        throw std::invalid_argument("Formation size must exceed z-score window");
    }

    WalkForwardResult result;
    
    const std::size_t agg_start = parameters.formation_size;
    const std::size_t agg_size = data.size() - agg_start;

    std::vector<std::int64_t> agg_timestamps;
    std::vector<double> agg_x;
    std::vector<double> agg_y;
    
    // Pre-fill with flat state.
    // Failed folds retain 0 signal, generating no exposure.
    std::vector<int> agg_signals(agg_size, 0);
    std::vector<double> agg_hedge_ratios(agg_size, 1.0);

    agg_timestamps.reserve(agg_size);
    agg_x.reserve(agg_size);
    agg_y.reserve(agg_size);

    for (std::size_t i = agg_start; i < data.size(); ++i) {
        agg_timestamps.push_back(data[i].timestamp);
        agg_x.push_back(data[i].x);
        agg_y.push_back(data[i].y);
    }

    std::size_t test_begin = parameters.formation_size;

    while (test_begin < data.size()) {
        const std::size_t test_end = std::min(test_begin + parameters.test_size, data.size());
        
        const std::size_t formation_begin = 0;
        const std::size_t formation_end = test_begin;

        const auto x = make_x_series(data, formation_begin, formation_end);
        const auto y = make_y_series(data, formation_begin, formation_end);

        const auto cointegration = quant::math::engle_granger(x, y, 0);
        const bool formation_passed = (cointegration.decision == quant::math::CointegrationDecision::Cointegrated);

        WalkForwardFold fold{
            formation_begin,
            formation_end,
            test_begin,
            test_end,
            formation_passed,
            std::nullopt
        };

        if (formation_passed) {
            // slice_end = test_end + 1 strictly to expose the boundary signal
            const std::size_t slice_end = std::min(test_end + 1, data.size());
            const auto fold_data = slice(data, 0, slice_end);
            
            const auto research = run_pairs_research(
                fold_data,
                parameters.strategy,
                test_begin
            );

            fold.performance = quant::risk::calculate_performance(
                research.backtest,
                periods_per_year
            );

            // Stitch target signals using strict timestamp alignment
            for (std::size_t t = test_begin; t < test_end; ++t) {
                const std::size_t out_idx = t - agg_start;
                const std::int64_t current_ts = data[t].timestamp;

                auto hr_it = std::lower_bound(
                    research.timestamps.begin(),
                    research.timestamps.end(),
                    current_ts
                );
                
                if (hr_it != research.timestamps.end() && *hr_it == current_ts) {
                    std::size_t idx = std::distance(research.timestamps.begin(), hr_it);
                    agg_hedge_ratios[out_idx] = research.hedge_ratios[idx];
                }

                if (t + 1 < data.size()) {
                    const std::int64_t next_ts = data[t + 1].timestamp;
                    auto bar_it = std::lower_bound(
                        research.backtest.bars.begin(),
                        research.backtest.bars.end(),
                        next_ts,
                        [](const auto& bar, std::int64_t target) {
                            return bar.timestamp < target;
                        }
                    );

                    if (bar_it != research.backtest.bars.end() && bar_it->timestamp == next_ts) {
                        agg_signals[out_idx] = bar_it->signal;
                    }
                }
            }
        }

        result.folds.push_back(std::move(fold));
        test_begin = test_end;
    }

    if (agg_size >= 2) {
        result.aggregate_backtest = quant::backtest::run_backtest(
            agg_timestamps,
            agg_x,
            agg_y,
            agg_signals,
            agg_hedge_ratios,
            parameters.strategy.backtest_parameters
        );

        result.aggregate_performance = quant::risk::calculate_performance(
            *result.aggregate_backtest,
            periods_per_year
        );
    }

    return result;
}

} // namespace quant::research