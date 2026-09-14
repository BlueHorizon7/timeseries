#include "quant/research/parameter_sensitivity.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    using namespace quant;

    data::AlignedSeries data;

    /*
     * Deterministically cointegrated synthetic pair.
     *
     * The oscillating spread creates repeated z-score
     * excursions, allowing different signal parameters
     * to actually affect the strategy.
     */
    for (std::size_t i = 0; i < 100; ++i) {
        const double x =
            100.0 + static_cast<double>(i);

        const double spread =
            2.0 *
            std::sin(
                2.0 *
                3.14159265358979323846 *
                static_cast<double>(i) /
                10.0
            );

        const double y =
            2.0 * x + spread;

        data.add(
            data::AlignedObservation{
                static_cast<std::int64_t>(
                    1000 + i * 86400
                ),
                x,
                y
            }
        );
    }

    research::WalkForwardParameters parameters;

    parameters.formation_size = 40;
    parameters.test_size = 20;

    parameters.strategy.hedge_ratio_window = 20;
    parameters.strategy.zscore_window = 20;

    parameters.strategy
        .signal_parameters
        .entry_zscore = 0.5;

    parameters.strategy
        .signal_parameters
        .exit_zscore = 0.1;

    parameters.strategy
        .backtest_parameters
        .initial_capital = 100000.0;

    parameters.strategy
        .backtest_parameters
        .gross_notional = 10000.0;

    /*
     * Zero explicit transaction/execution costs keep this
     * test focused on parameter sensitivity.
     */

    {
        const auto result =
            research::analyze_parameter_sensitivity(
                data,
                parameters,
                252.0,
                research::SensitivityParameter::
                    HedgeRatioWindow,
                {10.0, 20.0, 30.0}
            );

        assert(result.parameter ==
               research::SensitivityParameter::
                   HedgeRatioWindow);

        assert(result.points.size() == 3);

        for (const auto& point : result.points) {
            assert(
                std::isfinite(
                    point.performance.final_equity
                )
            );

            assert(
                point.performance.number_of_bars > 0
            );
        }
    }

    {
        const auto result =
            research::analyze_parameter_sensitivity(
                data,
                parameters,
                252.0,
                research::SensitivityParameter::
                    ZScoreWindow,
                {10.0, 20.0, 30.0}
            );

        assert(result.points.size() == 3);

        for (const auto& point : result.points) {
            assert(
                std::isfinite(
                    point.performance.sharpe_ratio
                )
            );
        }
    }

    {
        const auto result =
            research::analyze_parameter_sensitivity(
                data,
                parameters,
                252.0,
                research::SensitivityParameter::
                    EntryZScore,
                {0.5, 1.0, 1.5}
            );

        assert(result.points.size() == 3);
    }

    {
        const auto result =
            research::analyze_parameter_sensitivity(
                data,
                parameters,
                252.0,
                research::SensitivityParameter::
                    ExitZScore,
                {0.0, 0.1, 0.2}
            );

        assert(result.points.size() == 3);
    }

    {
        bool threw = false;

        try {
            (void)
                research::analyze_parameter_sensitivity(
                    data,
                    parameters,
                    252.0,
                    research::SensitivityParameter::
                        HedgeRatioWindow,
                    {1.0}
                );
        } catch (const std::invalid_argument&) {
            threw = true;
        }

        assert(threw);
    }

    std::cout
        << "Parameter sensitivity tests passed!\n";

    return 0;
}