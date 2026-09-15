#include "quant/research/placebo.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>

namespace {

quant::data::AlignedSeries make_synthetic_data() {
    quant::data::AlignedSeries data;

    constexpr std::size_t n = 500;

    std::uint64_t state =
        88172645463325252ULL;

    double random_walk = 100.0;
    double stationary_noise = 0.0;

    for (std::size_t i = 0; i < n; ++i) {
        state =
            state *
                2862933555777941757ULL +
            3037000493ULL;

        const double uniform =
            static_cast<double>(
                state >> 11
            ) *
            (1.0 / 9007199254740992.0);

        const double innovation =
            2.0 * uniform - 1.0;

        random_walk += innovation;

        stationary_noise =
            0.5 * stationary_noise +
            0.05 * innovation;

        const double x =
            random_walk;

        const double y =
            5.0 +
            2.0 * x +
            stationary_noise;

        data.add(
            quant::data::AlignedObservation{
                static_cast<std::int64_t>(i + 1),
                x,
                y
            }
        );
    }

    return data;
}

quant::research::WalkForwardParameters
make_parameters() {
    quant::research::WalkForwardParameters parameters{};

    parameters.formation_size = 350;
    parameters.test_size = 50;

    parameters.cointegration
        .automatic_lag_selection = true;

    parameters.cointegration
        .max_adf_lags = 8;

    parameters.cointegration
        .information_criterion =
            quant::math::InformationCriterion::AIC;

    parameters.strategy.hedge_ratio_window = 20;
    parameters.strategy.zscore_window = 20;

    parameters.strategy.signal_parameters
        .entry_zscore = 0.5;

    parameters.strategy.signal_parameters
        .exit_zscore = 0.1;

    parameters.strategy.backtest_parameters
        .initial_capital = 10000.0;

    parameters.strategy.backtest_parameters
        .gross_notional = 1000.0;

    parameters.strategy.backtest_parameters
        .transaction_costs.x_cost_rate = 0.0010;

    parameters.strategy.backtest_parameters
        .transaction_costs.y_cost_rate = 0.0010;

    return parameters;
}

} // namespace

int main() {
    const auto data =
        make_synthetic_data();

    const auto parameters =
        make_parameters();

    quant::research::PlaceboParameters placebo_parameters{};

    placebo_parameters.number_of_placebos = 100;
    placebo_parameters.random_seed = 123456789ULL;
    placebo_parameters.periods_per_year = 252.0;

    const auto result =
        quant::research::analyze_circular_shift_placebo(
            data,
            parameters,
            placebo_parameters
        );

    assert(
        result.number_of_placebos_requested == 100
    );

    assert(
        result.number_of_successful_placebos > 0
    );

    assert(
        result.number_of_successful_placebos <= 100
    );

    assert(
        result.placebo_total_returns.size() ==
        result.number_of_successful_placebos
    );

    assert(
        result.placebo_sharpes.size() ==
        result.number_of_successful_placebos
    );

    assert(
        std::isfinite(
            result.observed_total_return
        )
    );

    assert(
        std::isfinite(
            result.observed_sharpe_ratio
        )
    );

    assert(
        std::isfinite(
            result.empirical_p_value_total_return
        )
    );

    assert(
        std::isfinite(
            result.empirical_p_value_sharpe_ratio
        )
    );

    assert(
        result.empirical_p_value_total_return > 0.0
    );

    assert(
        result.empirical_p_value_total_return <= 1.0
    );

    assert(
        result.empirical_p_value_sharpe_ratio > 0.0
    );

    assert(
        result.empirical_p_value_sharpe_ratio <= 1.0
    );

    /*
        Reproducibility.
    */
    const auto repeat_result =
        quant::research::analyze_circular_shift_placebo(
            data,
            parameters,
            placebo_parameters
        );

    assert(
        result.placebo_total_returns ==
        repeat_result.placebo_total_returns
    );

    assert(
        result.placebo_sharpes ==
        repeat_result.placebo_sharpes
    );

    /*
        Different seed should normally generate a different
        placebo distribution.
    */
    placebo_parameters.random_seed =
        987654321ULL;

    const auto different_seed_result =
        quant::research::analyze_circular_shift_placebo(
            data,
            parameters,
            placebo_parameters
        );

    assert(
        result.placebo_total_returns !=
        different_seed_result.placebo_total_returns
    );

    /*
        Invalid number of placebos.
    */
    {
        auto invalid = placebo_parameters;
        invalid.number_of_placebos = 0;

        bool threw = false;

        try {
            (void)
                quant::research::
                    analyze_circular_shift_placebo(
                        data,
                        parameters,
                        invalid
                    );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        Invalid periods per year.
    */
    {
        auto invalid = placebo_parameters;
        invalid.periods_per_year = 0.0;

        bool threw = false;

        try {
            (void)
                quant::research::
                    analyze_circular_shift_placebo(
                        data,
                        parameters,
                        invalid
                    );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        Too little data.
    */
    {
        quant::data::AlignedSeries tiny_data;

        tiny_data.add(
            quant::data::AlignedObservation{
                1,
                100.0,
                200.0
            }
        );

        bool threw = false;

        try {
            (void)
                quant::research::
                    analyze_circular_shift_placebo(
                        tiny_data,
                        parameters,
                        placebo_parameters
                    );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    std::cout
        << "Placebo tests passed!\n";

    return 0;
}