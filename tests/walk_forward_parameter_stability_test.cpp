#include "quant/research/walk_forward_parameter_stability.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

using namespace quant;

namespace {

data::AlignedSeries make_synthetic_data() {
    data::AlignedSeries data;

    constexpr std::size_t n = 500;

    /*
        Deterministic pseudo-random generator.

        This is the same data-generating process used by the
        passing formation_test.cpp.

        Integrated X process:

            X_t = X_(t-1) + u_t

        Stationary disturbance:

            e_t = 0.5 e_(t-1) + 0.05 u_t

        Cointegrating relationship:

            Y_t = 5 + 2 X_t + e_t
    */
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

        const double x_value =
            random_walk;

        const double y_value =
            5.0 +
            2.0 * x_value +
            stationary_noise;

        data.add(
            data::AlignedObservation{
                static_cast<std::int64_t>(i + 1),
                x_value,
                y_value
            }
        );
    }

    return data;
}

research::WalkForwardParameters make_baseline_parameters() {
    research::WalkForwardParameters parameters{};

    /*
        Use the same formation size that is already proven to
        pass the formation-stage cointegration test.
    */
    parameters.formation_size = 350;
    parameters.test_size = 50;

    parameters.cointegration.automatic_lag_selection = true;

    parameters.cointegration.max_adf_lags = 8;

    parameters.cointegration.information_criterion =
        quant::math::InformationCriterion::AIC;

    parameters.strategy.hedge_ratio_window = 20;
    parameters.strategy.zscore_window = 20;

    parameters.strategy.signal_parameters.entry_zscore = 0.5;
    parameters.strategy.signal_parameters.exit_zscore = 0.1;

    parameters.strategy.backtest_parameters.initial_capital =
        10000.0;

    parameters.strategy.backtest_parameters.gross_notional =
        1000.0;

    /*
        Keep trading costs explicit.
    */
    parameters.strategy.backtest_parameters
        .transaction_costs.x_cost_rate = 0.0010;

    parameters.strategy.backtest_parameters
        .transaction_costs.y_cost_rate = 0.0010;

    return parameters;
}

void assert_valid_stability_point(
    const research::WalkForwardParameterStabilityPoint& point,
    research::SensitivityParameter expected_parameter,
    double minimum_value,
    double maximum_value,
    std::size_t expected_number_of_parameters,
    std::size_t number_of_folds
) {
    assert(
        point.parameter ==
        expected_parameter
    );

    assert(
        point.parameter_value >=
        minimum_value
    );

    assert(
        point.parameter_value <=
        maximum_value
    );

    /*
        At least one fold must provide an OOS
        performance observation.
    */
    assert(
        point.number_of_observed_folds > 0
    );

    assert(
        point.number_of_observed_folds <=
        number_of_folds
    );

    assert(
        point.fold_total_returns.size() ==
        point.number_of_observed_folds
    );

    assert(
        point.fold_sharpes.size() ==
        point.number_of_observed_folds
    );

    assert(
        std::isfinite(
            point.mean_total_return
        )
    );

    assert(
        std::isfinite(
            point.standard_deviation_total_return
        )
    );

    assert(
        point.standard_deviation_total_return >=
        0.0
    );

    assert(
        std::isfinite(
            point.mean_sharpe
        )
    );

    assert(
        std::isfinite(
            point.standard_deviation_sharpe
        )
    );

    assert(
        point.standard_deviation_sharpe >=
        0.0
    );

    /*
        Rank 1 is best, so every mean rank must lie
        inside [1, number_of_parameters].
    */
    assert(
        std::isfinite(
            point.mean_total_return_rank
        )
    );

    assert(
        point.mean_total_return_rank >=
        1.0
    );

    assert(
        point.mean_total_return_rank <=
        static_cast<double>(
            expected_number_of_parameters
        )
    );

    assert(
        std::isfinite(
            point.standard_deviation_total_return_rank
        )
    );

    assert(
        point.standard_deviation_total_return_rank >=
        0.0
    );

    assert(
        std::isfinite(
            point.mean_sharpe_rank
        )
    );

    assert(
        point.mean_sharpe_rank >=
        1.0
    );

    assert(
        point.mean_sharpe_rank <=
        static_cast<double>(
            expected_number_of_parameters
        )
    );

    assert(
        std::isfinite(
            point.standard_deviation_sharpe_rank
        )
    );

    assert(
        point.standard_deviation_sharpe_rank >=
        0.0
    );

    for (const double value :
         point.fold_total_returns) {
        assert(std::isfinite(value));
    }

    for (const double value :
         point.fold_sharpes) {
        assert(std::isfinite(value));
    }
}

void assert_throws_for_invalid_window(
    const data::AlignedSeries& data,
    const research::WalkForwardParameters& parameters
) {
    bool threw = false;

    try {
        (void)
            research::
                analyze_walk_forward_parameter_stability(
                    data,
                    parameters,
                    252.0,
                    research::SensitivityParameter::
                        HedgeRatioWindow,
                    {1.0}
                );
    } catch (...) {
        threw = true;
    }

    assert(threw);
}

void assert_throws_for_invalid_entry_zscore(
    const data::AlignedSeries& data,
    const research::WalkForwardParameters& parameters
) {
    bool threw = false;

    try {
        (void)
            research::
                analyze_walk_forward_parameter_stability(
                    data,
                    parameters,
                    252.0,
                    research::SensitivityParameter::
                        EntryZScore,
                    {0.0}
                );
    } catch (...) {
        threw = true;
    }

    assert(threw);
}

void assert_throws_for_empty_values(
    const data::AlignedSeries& data,
    const research::WalkForwardParameters& parameters
) {
    bool threw = false;

    try {
        (void)
            research::
                analyze_walk_forward_parameter_stability(
                    data,
                    parameters,
                    252.0,
                    research::SensitivityParameter::
                        EntryZScore,
                    {}
                );
    } catch (...) {
        threw = true;
    }

    assert(threw);
}

void assert_throws_for_invalid_periods_per_year(
    const data::AlignedSeries& data,
    const research::WalkForwardParameters& parameters
) {
    bool threw = false;

    try {
        (void)
            research::
                analyze_walk_forward_parameter_stability(
                    data,
                    parameters,
                    0.0,
                    research::SensitivityParameter::
                        EntryZScore,
                    {0.5, 1.0, 1.5}
                );
    } catch (...) {
        threw = true;
    }

    assert(threw);
}

} // namespace

int main() {
    const data::AlignedSeries dummy_data =
        make_synthetic_data();

    const research::WalkForwardParameters parameters =
        make_baseline_parameters();

    /*
        First verify the baseline walk-forward.

        With 500 observations, a formation size of 350,
        and test size of 50, there are:

            floor((500 - 350) / 50) = 3

        folds.
    */
    const auto baseline_result =
        research::run_walk_forward(
            dummy_data,
            parameters,
            252.0
        );

    assert(
        baseline_result.folds.size() == 3
    );

    assert(
        baseline_result.aggregate_backtest.has_value()
    );

    assert(
        baseline_result.aggregate_performance.has_value()
    );

    /*
        Diagnostic output makes formation-stage failures
        immediately visible.
    */
    std::cout
        << "Baseline folds: "
        << baseline_result.folds.size()
        << '\n';

    std::size_t successful_folds = 0;

    for (std::size_t i = 0;
         i < baseline_result.folds.size();
         ++i) {

        const auto& fold =
            baseline_result.folds[i];

        std::cout
            << "Fold "
            << i
            << ": formation_passed="
            << fold.formation_passed
            << ", performance="
            << fold.performance.has_value()
            << '\n';

        if (fold.formation_passed &&
            fold.performance.has_value()) {
            ++successful_folds;
        }
    }

    /*
        The stability analysis cannot be meaningful when the
        baseline produces no successful OOS folds.
    */
    assert(
        successful_folds > 0
    );

    /*
        Hedge-ratio-window stability.
    */
    const auto hedge_result =
        research::
            analyze_walk_forward_parameter_stability(
                dummy_data,
                parameters,
                252.0,
                research::SensitivityParameter::
                    HedgeRatioWindow,
                {5.0, 10.0, 15.0}
            );

    assert(
        hedge_result.parameter ==
        research::SensitivityParameter::
            HedgeRatioWindow
    );

    assert(
        hedge_result.points.size() == 3
    );

    assert(
        hedge_result.number_of_folds ==
        baseline_result.folds.size()
    );

    for (const auto& point :
         hedge_result.points) {

        assert_valid_stability_point(
            point,
            research::SensitivityParameter::
                HedgeRatioWindow,
            5.0,
            15.0,
            hedge_result.points.size(),
            hedge_result.number_of_folds
        );
    }

    /*
        Entry-z-score stability.
    */
    const auto entry_result =
        research::
            analyze_walk_forward_parameter_stability(
                dummy_data,
                parameters,
                252.0,
                research::SensitivityParameter::
                    EntryZScore,
                {0.5, 1.0, 1.5}
            );

    assert(
        entry_result.parameter ==
        research::SensitivityParameter::
            EntryZScore
    );

    assert(
        entry_result.points.size() == 3
    );

    assert(
        entry_result.number_of_folds ==
        baseline_result.folds.size()
    );

    for (const auto& point :
         entry_result.points) {

        assert_valid_stability_point(
            point,
            research::SensitivityParameter::
                EntryZScore,
            0.5,
            1.5,
            entry_result.points.size(),
            entry_result.number_of_folds
        );
    }

    /*
        Invalid hedge-ratio window.
    */
    assert_throws_for_invalid_window(
        dummy_data,
        parameters
    );

    /*
        Invalid entry z-score.
    */
    assert_throws_for_invalid_entry_zscore(
        dummy_data,
        parameters
    );

    /*
        Empty parameter grid.
    */
    assert_throws_for_empty_values(
        dummy_data,
        parameters
    );

    /*
        Invalid annualization frequency.
    */
    assert_throws_for_invalid_periods_per_year(
        dummy_data,
        parameters
    );

    std::cout
        << "Walk-forward parameter stability tests passed!\n";

    return 0;
}