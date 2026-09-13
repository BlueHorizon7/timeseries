#include "quant/research/pairs_research.hpp"

#include "quant/math/rolling_regression.hpp"
#include "quant/math/rolling_spread.hpp"
#include "quant/math/rolling_zscore.hpp"
#include "quant/math/series.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace quant::research {

namespace {

quant::math::Series
make_x_series(
    const quant::data::AlignedSeries& prices
) {
    quant::math::Series result;

    result.reserve(prices.size());

    for (const auto& observation : prices) {
        result.add(
            quant::math::Observation{
                observation.timestamp,
                observation.x
            }
        );
    }

    return result;
}

quant::math::Series
make_y_series(
    const quant::data::AlignedSeries& prices
) {
    quant::math::Series result;

    result.reserve(prices.size());

    for (const auto& observation : prices) {
        result.add(
            quant::math::Observation{
                observation.timestamp,
                observation.y
            }
        );
    }

    return result;
}

/*
 * Find the price index corresponding to a timestamp.
 *
 * AlignedSeries is strictly increasing in timestamp, so
 * binary search gives O(log n) lookup.
 */
std::size_t find_price_index(
    const quant::data::AlignedSeries& prices,
    std::int64_t timestamp
) {
    std::size_t left = 0;
    std::size_t right = prices.size();

    while (left < right) {
        const std::size_t middle =
            left + (right - left) / 2;

        if (prices[middle].timestamp < timestamp) {
            left = middle + 1;
        } else {
            right = middle;
        }
    }

    if (left >= prices.size() ||
        prices[left].timestamp != timestamp) {

        throw std::logic_error(
            "Could not map research timestamp "
            "to price series"
        );
    }

    return left;
}

} // namespace

PairsResearchResult run_pairs_research(
    const quant::data::AlignedSeries& prices,
    const PairsResearchParameters& parameters,
    std::size_t trading_begin
) {
    if (prices.size() < 3) {
        throw std::invalid_argument(
            "Pairs research requires at least three observations"
        );
    }

    if (trading_begin >= prices.size()) {
        throw std::invalid_argument(
            "Trading boundary must be inside price series"
        );
    }

    if (parameters.hedge_ratio_window < 2) {
        throw std::invalid_argument(
            "Hedge ratio window must be at least two"
        );
    }

    if (parameters.zscore_window < 2) {
        throw std::invalid_argument(
            "Z-score window must be at least two"
        );
    }

    if (prices.size() <=
        parameters.hedge_ratio_window) {

        throw std::invalid_argument(
            "Not enough observations for hedge ratio window"
        );
    }

    const auto x =
        make_x_series(prices);

    const auto y =
        make_y_series(prices);

    /*
     * ---------------------------------------------------------
     * 1. Rolling hedge ratios
     * ---------------------------------------------------------
     *
     * At timestamp t:
     *
     *     model_t
     *
     * is estimated from:
     *
     *     [t-window, ..., t-1]
     *
     * Therefore observation t is never used to estimate
     * the model applied at t.
     */

    const auto rolling_models =
        quant::math::rolling_ols(
            x,
            y,
            parameters.hedge_ratio_window
        );

    /*
     * ---------------------------------------------------------
     * 2. Causal rolling spread
     * ---------------------------------------------------------
     */

    const auto spread =
        quant::math::rolling_spread(
            x,
            y,
            parameters.hedge_ratio_window
        );

    /*
     * ---------------------------------------------------------
     * 3. Causal rolling z-score
     * ---------------------------------------------------------
     *
     * rolling_zscore() preserves the timestamp of the
     * spread observation being evaluated.
     */

    const auto zscores =
        quant::math::rolling_zscore(
            spread,
            parameters.zscore_window
        );

    /*
     * ---------------------------------------------------------
     * 4. Stateful signals
     * ---------------------------------------------------------
     */

    const auto signal_series =
        quant::strategy::generate_signals(
            zscores,
            parameters.signal_parameters
        );

    if (signal_series.size() != zscores.size()) {
        throw std::logic_error(
            "Signal and z-score series have different sizes"
        );
    }

    /*
     * ---------------------------------------------------------
     * 5. Locate the first tradable z-score using timestamps.
     * ---------------------------------------------------------
     *
     * Do NOT reconstruct the relationship using assumed
     * vector offsets.
     *
     * The timestamp is the source of truth.
     */

    const auto trading_timestamp =
        prices[trading_begin].timestamp;

    std::size_t first_zscore_index = 0;

    while (
        first_zscore_index < zscores.size() &&
        zscores[first_zscore_index].timestamp <
            trading_timestamp
    ) {
        ++first_zscore_index;
    }

    if (first_zscore_index >= zscores.size()) {
        throw std::invalid_argument(
            "Trading period begins before the first "
            "available z-score"
        );
    }

    PairsResearchResult result;

    const std::size_t output_size =
        zscores.size() -
        first_zscore_index;

    result.timestamps.reserve(output_size);
    result.x_prices.reserve(output_size);
    result.y_prices.reserve(output_size);
    result.hedge_ratios.reserve(output_size);
    result.spreads.reserve(output_size);
    result.zscores.reserve(output_size);
    result.signals.reserve(output_size);

    /*
     * ---------------------------------------------------------
     * 6. Assemble research observations.
     * ---------------------------------------------------------
     */

    for (std::size_t i = first_zscore_index;
         i < zscores.size();
         ++i) {

        const auto& zscore =
            zscores[i];

        const auto& signal =
            signal_series[i];

        const std::int64_t timestamp =
            zscore.timestamp;

        /*
         * Find the exact aligned price observation
         * corresponding to this research timestamp.
         */

        const std::size_t price_index =
            find_price_index(
                prices,
                timestamp
            );

        /*
         * Locate the rolling model by its explicit
         * timestamp index.
         */

        const auto& model =
            rolling_models[
                price_index -
                parameters.hedge_ratio_window
            ];

        /*
         * Verify that the rolling model really belongs
         * to this price observation.
         */

        if (model.timestamp_index != price_index) {
            throw std::logic_error(
                "Rolling model index does not match "
                "research timestamp"
            );
        }

        /*
         * Find the spread observation corresponding to
         * the z-score timestamp.
         *
         * The rolling spread begins at the same point as
         * rolling OLS.
         */

        const std::size_t spread_index =
            price_index -
            parameters.hedge_ratio_window;

        if (spread_index >= spread.size()) {
            throw std::logic_error(
                "Spread index exceeds available results"
            );
        }

        if (spread[spread_index].timestamp !=
            timestamp) {

            throw std::logic_error(
                "Spread timestamp does not match "
                "z-score timestamp"
            );
        }

        /*
         * Final timestamp invariants.
         */

        if (prices[price_index].timestamp !=
            timestamp) {

            throw std::logic_error(
                "Price timestamp does not match "
                "z-score timestamp"
            );
        }

        result.timestamps.push_back(
            timestamp
        );

        result.x_prices.push_back(
            prices[price_index].x
        );

        result.y_prices.push_back(
            prices[price_index].y
        );

        result.hedge_ratios.push_back(
            model.model.slope
        );

        result.spreads.push_back(
            spread[spread_index].value
        );

        result.zscores.push_back(
            zscore.value
        );

        result.signals.push_back(
            static_cast<int>(
                signal.value
            )
        );
    }

    if (result.timestamps.size() < 2) {
        throw std::invalid_argument(
            "Pairs research requires at least "
            "two tradable observations"
        );
    }

    /*
     * ---------------------------------------------------------
     * 7. Backtest
     * ---------------------------------------------------------
     *
     * Signal at t earns the return from t -> t+1.
     */

    result.backtest =
        quant::backtest::run_backtest(
            result.timestamps,
            result.x_prices,
            result.y_prices,
            result.signals,
            result.hedge_ratios,
            parameters.backtest_parameters
        );

    return result;
}

} // namespace quant::research