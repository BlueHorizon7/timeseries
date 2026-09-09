#include "quant/data/aligned_series.hpp"
#include "quant/research/pairs_research.hpp"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>

int main() {
    using namespace quant::data;
    using namespace quant::research;

    AlignedSeries prices;

    /*
     * A deterministic synthetic pair with small deviations.
     *
     * Y is approximately 2X, but not perfectly so.
     * This prevents the spread from having zero variance.
     */
    const double x_values[] = {
        100.0,
        101.0,
        102.0,
        103.0,
        104.0,
        105.0,
        106.0,
        107.0,
        108.0,
        109.0,
        110.0,
        111.0,
        112.0,
        113.0,
        114.0,
        115.0,
        116.0,
        117.0,
        118.0,
        119.0
    };

    const double y_values[] = {
        200.0,
        202.2,
        203.8,
        206.3,
        207.7,
        210.4,
        211.6,
        214.2,
        215.9,
        218.3,
        219.7,
        222.4,
        223.6,
        226.1,
        227.8,
        230.2,
        231.9,
        234.3,
        235.7,
        238.2
    };

    for (std::size_t i = 0; i < 20; ++i) {
        prices.add(
            AlignedObservation{
                static_cast<std::int64_t>(
                    1000 + i * 1000
                ),
                x_values[i],
                y_values[i]
            }
        );
    }

    PairsResearchParameters parameters;

    parameters.hedge_ratio_window = 5;
    parameters.zscore_window = 3;

    parameters.signal_parameters =
        quant::strategy::SignalParameters{
            2.0,
            0.5
        };

    parameters.backtest_parameters =
        quant::backtest::BacktestParameters{
            100000.0,
            10000.0,
            quant::portfolio::TransactionCostParameters{
                0.001,
                0.001
            }
        };

    const auto result =
        run_pairs_research(
            prices,
            parameters
        );

    /*
     * 20 observations.
     *
     * Hedge-ratio warm-up:
     *
     *     5 observations
     *
     * Z-score warm-up:
     *
     *     3 observations
     *
     * First usable z-score:
     *
     *     index = 5 + 3 - 1
     *           = 7
     *
     * Therefore:
     *
     *     20 - 7 = 13
     *
     * usable research observations.
     */
    assert(result.timestamps.size() == 13);
    assert(result.x_prices.size() == 13);
    assert(result.y_prices.size() == 13);
    assert(result.hedge_ratios.size() == 13);
    assert(result.spreads.size() == 13);
    assert(result.zscores.size() == 13);
    assert(result.signals.size() == 13);

    /*
     * The backtest needs a following price observation
     * to calculate the return after the signal.
     *
     * Therefore 13 research observations produce
     * 12 backtest bars.
     */
    assert(result.backtest.bars.size() == 12);

    /*
     * The synthetic relationship is approximately:
     *
     *     Y = 2X
     *
     * so the estimated hedge ratio should remain close
     * to 2.
     */
    for (const double beta : result.hedge_ratios) {
        assert(std::isfinite(beta));
        assert(std::abs(beta - 2.0) < 0.1);
    }

    /*
     * Every spread must be finite.
     */
    for (const double spread : result.spreads) {
        assert(std::isfinite(spread));
    }

    /*
     * Every z-score must be finite.
     */
    for (const double zscore : result.zscores) {
        assert(std::isfinite(zscore));
    }

    /*
     * Signals must always belong to:
     *
     *     {-1, 0, +1}
     */
    for (const int signal : result.signals) {
        assert(
            signal == -1 ||
            signal == 0 ||
            signal == 1
        );
    }

    /*
     * Every backtest equity value must be finite.
     */
    for (const auto& bar : result.backtest.bars) {
        assert(std::isfinite(bar.gross_pnl));
        assert(std::isfinite(bar.transaction_cost));
        assert(std::isfinite(bar.net_pnl));
        assert(std::isfinite(bar.equity));
    }

    /*
     * Initial capital is preserved unless the backtest
     * produces P&L or transaction costs.
     */
    assert(
        result.backtest.bars.front().equity != 0.0
    );

    return 0;
}