#include "quant/research/walk_forward.hpp"
#include "quant/portfolio/transaction_cost.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>

using namespace quant;

int main() {
    data::AlignedSeries dummy_data;

    /*
        Create an oscillating, deterministically cointegrated
        synthetic pair.

            Y_t = 2 X_t + spread_t

        The oscillating spread is intended to produce
        non-flat positions around the fold boundary.
    */
    for (std::size_t i = 0; i < 60; ++i) {
        const double x =
            100.0 + static_cast<double>(i);

        const double spread =
            2.0 *
            std::sin(
                2.0 *
                3.14159265358979323846 *
                static_cast<double>(i) /
                4.0
            );

        const double y =
            2.0 * x + spread;

        dummy_data.add(
            data::AlignedObservation{
                static_cast<std::int64_t>(
                    1000 + i * 86400
                ),
                x,
                y
            }
        );
    }

    research::WalkForwardParameters params;

    params.formation_size = 40;
    params.test_size = 10;

    params.strategy.hedge_ratio_window = 20;
    params.strategy.zscore_window = 20;

    params.strategy.signal_parameters.entry_zscore =
        0.5;

    params.strategy.signal_parameters.exit_zscore =
        0.1;

    params.strategy.backtest_parameters.initial_capital =
        10000.0;

    params.strategy.backtest_parameters.gross_notional =
        1000.0;

    /*
        Explicit nonzero transaction costs.
    */
    params.strategy.backtest_parameters
        .transaction_costs.x_cost_rate = 0.0010;

    params.strategy.backtest_parameters
        .transaction_costs.y_cost_rate = 0.0010;

    const auto result =
        research::run_walk_forward(
            dummy_data,
            params,
            252.0
        );

    assert(result.folds.size() == 2);
    assert(result.aggregate_backtest.has_value());
    assert(result.aggregate_performance.has_value());

    const auto& agg =
        *result.aggregate_backtest;

    /*
        60 total observations
        - 40 formation observations
        = 20 OOS observations

        Therefore there are 19 OOS intervals.
    */
    assert(agg.bars.size() == 19);

    /*
        OOS bars must remain strictly chronological.
    */
    for (std::size_t i = 1;
         i < agg.bars.size();
         ++i) {

        assert(
            agg.bars[i].timestamp >
            agg.bars[i - 1].timestamp
        );
    }

    /*
        Equity must be continuous.
    */
    double expected_equity =
        params.strategy.backtest_parameters.initial_capital;

    for (std::size_t i = 0;
         i < agg.bars.size();
         ++i) {

        expected_equity +=
            agg.bars[i].net_pnl;

        assert(
            std::abs(
                agg.bars[i].equity -
                expected_equity
            ) < 1e-8
        );
    }

    /*
        Terminal liquidation is represented separately,
        not as another BacktestBar.
    */
    assert(agg.liquidation_cost >= 0.0);

    /*
        Verify the interior fold boundary.

        Fold 1:
            [40, 50)

        Fold 2:
            [50, 60)

        Because a signal at t produces the position
        during t -> t+1, the relevant aggregate bar
        around the seam is the bar ending at observation 51.
    */
    const std::int64_t post_boundary_timestamp =
        dummy_data[51].timestamp;

    bool verified_boundary = false;

    for (std::size_t i = 1;
         i < agg.bars.size();
         ++i) {

        if (agg.bars[i].timestamp !=
            post_boundary_timestamp) {

            continue;
        }

        verified_boundary = true;

        const auto& previous_bar =
            agg.bars[i - 1];

        const auto& current_bar =
            agg.bars[i];

        /*
            The position crossing the fold seam must
            actually be non-flat. This prevents the
            test from passing trivially with cash.
        */
        assert(
            previous_bar.position.x_notional != 0.0 ||
            previous_bar.position.y_notional != 0.0
        );

        /*
            Transaction cost must equal the actual
            change from the previous position to the
            current position.

            An artificial liquidation/re-entry would
            produce an incorrect cost here.
        */
        const double expected_cost =
            portfolio::transaction_cost(
                previous_bar.position,
                current_bar.position,
                params.strategy
                    .backtest_parameters
                    .transaction_costs
            );

        assert(
            std::abs(
                current_bar.transaction_cost -
                expected_cost
            ) < 1e-8
        );
    }

    assert(verified_boundary);

    /*
        Any fold that fails formation must remain flat
        throughout its OOS period.
    */
    for (const auto& fold : result.folds) {
        if (!fold.formation_passed) {
            std::size_t start_idx =
                fold.test_begin -
                params.formation_size;

            std::size_t end_idx =
                fold.test_end -
                params.formation_size;

            end_idx =
                std::min(
                    end_idx,
                    agg.bars.size()
                );

            for (std::size_t i = start_idx;
                 i < end_idx;
                 ++i) {

                assert(
                    agg.bars[i]
                        .position
                        .x_notional == 0.0
                );

                assert(
                    agg.bars[i]
                        .position
                        .y_notional == 0.0
                );
            }
        }
    }

    /*
        Aggregate performance must use the unified
        backtest's final equity after terminal liquidation.
    */
    const double final_equity_before_liquidation =
        agg.bars.back().equity;

    const double final_equity_after_liquidation =
        final_equity_before_liquidation -
        agg.liquidation_cost;

    assert(
        std::abs(
            result.aggregate_performance
                ->final_equity -
            final_equity_after_liquidation
        ) < 1e-8
    );

    std::cout
        << "Walk-forward continuous OOS aggregation "
           "tests passed!\n";

    return 0;
}