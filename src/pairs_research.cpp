#include "quant/research/pairs_research.hpp"

#include "quant/math/rolling_regression.hpp"
#include "quant/math/rolling_spread.hpp"
#include "quant/math/rolling_zscore.hpp"
#include "quant/math/series.hpp"

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

} // namespace

PairsResearchResult run_pairs_research(
    const quant::data::AlignedSeries& prices,
    const PairsResearchParameters& parameters
) {
    if (prices.size() < 3) {
        throw std::invalid_argument(
            "Pairs research requires at least three observations"
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

    if (prices.size() <= parameters.hedge_ratio_window) {
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
     * 1. Estimate rolling hedge ratios.
     * ---------------------------------------------------------
     *
     * Model:
     *
     *     Y_t = alpha_t + beta_t X_t + epsilon_t
     *
     * The model at t uses observations strictly before t.
     */
    const auto rolling_models =
        quant::math::rolling_ols(
            x,
            y,
            parameters.hedge_ratio_window
        );

    /*
     * ---------------------------------------------------------
     * 2. Construct the causal rolling spread.
     * ---------------------------------------------------------
     *
     * S_t =
     *     Y_t - alpha_hat_t - beta_hat_t X_t
     *
     * where the parameters were estimated from prior data.
     */
    const auto spread =
        quant::math::rolling_spread(
            x,
            y,
            parameters.hedge_ratio_window
        );

    /*
     * ---------------------------------------------------------
     * 3. Normalize the spread using a rolling z-score.
     * ---------------------------------------------------------
     *
     * The z-score window itself also excludes the current
     * observation.
     */
    const auto zscores =
        quant::math::rolling_zscore(
            spread,
            parameters.zscore_window
        );

    /*
     * ---------------------------------------------------------
     * 4. Generate stateful trading signals.
     * ---------------------------------------------------------
     */
    const auto signal_series =
        quant::strategy::generate_signals(
            zscores,
            parameters.signal_parameters
        );

    PairsResearchResult result;

    /*
     * zscores and signal_series have identical timestamps.
     */
    const std::size_t n =
        zscores.size();

    result.timestamps.reserve(n);
    result.x_prices.reserve(n);
    result.y_prices.reserve(n);
    result.hedge_ratios.reserve(n);
    result.spreads.reserve(n);
    result.zscores.reserve(n);
    result.signals.reserve(n);

    /*
     * rolling_ols starts at index hedge_ratio_window.
     *
     * rolling_spread therefore corresponds to:
     *
     *     rolling_models[0] -> timestamp hedge_ratio_window
     *
     * rolling z-score removes another zscore_window-1
     * observations before producing its first output.
     *
     * Therefore the first z-score corresponds to:
     *
     *     rolling_models[zscore_window - 1]
     */
    const std::size_t model_offset =
        parameters.zscore_window - 1;

    if (model_offset >= rolling_models.size()) {
        throw std::logic_error(
            "Insufficient rolling regression results for z-score series"
        );
    }

    for (std::size_t i = 0; i < n; ++i) {
        const auto& zscore =
            zscores[i];

        const auto& signal =
            signal_series[i];

        const std::size_t model_index =
            model_offset + i;

        if (model_index >= rolling_models.size()) {
            throw std::logic_error(
                "Rolling model and z-score timestamps are misaligned"
            );
        }

        const std::size_t price_index =
            parameters.hedge_ratio_window +
            parameters.zscore_window -
            1 +
            i;

        if (price_index >= prices.size()) {
            throw std::logic_error(
                "Research output exceeds price series"
            );
        }

        result.timestamps.push_back(
            zscore.timestamp
        );

        result.x_prices.push_back(
            prices[price_index].x
        );

        result.y_prices.push_back(
            prices[price_index].y
        );

        result.hedge_ratios.push_back(
            rolling_models[model_index].model.slope
        );

        result.spreads.push_back(
            spread[
                parameters.zscore_window - 1 + i
            ].value
        );

        result.zscores.push_back(
            zscore.value
        );

        result.signals.push_back(
            static_cast<int>(signal.value)
        );
    }

    if (result.timestamps.size() < 2) {
        throw std::invalid_argument(
            "Pairs research requires at least two tradable observations"
        );
    }

    /*
     * Backtest.
     *
     * Signal at t is applied to the interval
     *
     *     t -> t+1
     *
     * so the current signal never earns the return that
     * generated that signal.
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