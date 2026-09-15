#include "quant/research/experiment.hpp"

#include <cassert>
#include <iostream>

namespace {

quant::research::ResearchExperiment
make_experiment() {
    quant::research::ResearchExperiment experiment{};

    experiment.experiment_id =
        "pairs_001";

    experiment.description =
        "Synthetic pairs research experiment";

    experiment.periods_per_year =
        252.0;

    experiment.walk_forward_parameters
        .formation_size = 350;

    experiment.walk_forward_parameters
        .test_size = 50;

    experiment.bootstrap_parameters
        .number_of_resamples = 500;

    experiment.bootstrap_parameters
        .block_length = 5;

    experiment.bootstrap_parameters
        .random_seed = 123456789ULL;

    experiment.bootstrap_parameters
        .confidence_level = 0.95;

    experiment.bootstrap_parameters
        .periods_per_year = 252.0;

    experiment.placebo_parameters
        .number_of_placebos = 500;

    experiment.placebo_parameters
        .random_seed = 123456789ULL;

    experiment.placebo_parameters
        .periods_per_year = 252.0;

    experiment.robustness_criteria
        .minimum_break_even_cost_multiplier = 1.0;

    experiment.robustness_criteria
        .minimum_profitable_subperiod_fraction = 0.50;

    experiment.robustness_criteria
        .minimum_profitable_parameter_fraction = 0.50;

    experiment.robustness_criteria
        .minimum_profitable_walk_forward_parameter_fraction = 0.50;

    experiment.robustness_criteria
        .minimum_bootstrap_sharpe_lower_bound = 0.0;

    experiment.robustness_criteria
        .maximum_placebo_sharpe_p_value = 0.05;

    experiment.robustness_criteria
        .minimum_successful_placebos = 100;

    experiment.robustness_criteria
        .require_all_diagnostics = true;

    return experiment;
}

quant::research::WalkForwardResult
make_walk_forward_result() {
    quant::research::WalkForwardResult result{};

    quant::research::WalkForwardFold fold{};

    fold.formation_begin = 0;
    fold.formation_end = 350;
    fold.test_begin = 350;
    fold.test_end = 400;
    fold.formation_passed = true;

    result.folds.push_back(fold);

    quant::backtest::BacktestResult backtest{};

    quant::backtest::BacktestBar bar{};

    bar.timestamp = 1;
    bar.x_price = 100.0;
    bar.y_price = 200.0;
    bar.signal = 0;
    bar.position = {};
    bar.gross_pnl = 100.0;
    bar.transaction_cost = 0.0;
    bar.execution_cost = 0.0;
    bar.net_pnl = 100.0;
    bar.equity = 10100.0;

    backtest.bars.push_back(bar);

    backtest.liquidation_cost = 0.0;

    result.aggregate_backtest =
        backtest;

    quant::risk::PerformanceMetrics performance{};

    performance.initial_equity = 10000.0;
    performance.final_equity = 10100.0;
    performance.total_pnl = 100.0;
    performance.total_return = 0.01;
    performance.annualized_return = 0.01;
    performance.annualized_volatility = 0.10;
    performance.sharpe_ratio = 1.0;
    performance.maximum_drawdown = 0.0;
    performance.maximum_drawdown_pct = 0.0;
    performance.number_of_bars = 1;
    performance.number_of_position_changes = 0;

    result.aggregate_performance =
        performance;

    return result;
}

quant::research::CostSensitivityResult
make_cost_result() {
    quant::research::CostSensitivityResult result{};

    result.points.resize(2);

    result.points[0].cost_multiplier = 1.0;
    result.points[0].performance.total_return = 0.10;

    result.points[1].cost_multiplier = 2.0;
    result.points[1].performance.total_return = 0.02;

    result.break_even_found = true;
    result.break_even_cost_multiplier = 2.0;

    return result;
}

quant::research::ParameterSensitivityResult
make_parameter_result() {
    quant::research::ParameterSensitivityResult result{};

    result.parameter =
        quant::research::SensitivityParameter::EntryZScore;

    result.points.resize(2);

    for (std::size_t i = 0; i < 2; ++i) {
        result.points[i].parameter =
            quant::research::SensitivityParameter::EntryZScore;

        result.points[i].parameter_value =
            0.5 +
            static_cast<double>(i);

        result.points[i].performance.total_return =
            0.10;
    }

    return result;
}

quant::research::SubperiodStabilityResult
make_subperiod_result() {
    quant::research::SubperiodStabilityResult result{};

    result.periods.resize(2);

    result.profitable_periods = 2;
    result.losing_periods = 0;

    result.best_total_return = 0.10;
    result.worst_sharpe = 1.0;
    result.best_sharpe = 1.2;

    for (std::size_t i = 0; i < 2; ++i) {
        result.periods[i].begin_bar =
            i * 10;

        result.periods[i].end_bar =
            (i + 1) * 10;

        result.periods[i]
            .performance.total_return = 0.10;
    }

    return result;
}

quant::research::WalkForwardParameterStabilityResult
make_walk_forward_parameter_result() {
    quant::research::WalkForwardParameterStabilityResult result{};

    result.parameter =
        quant::research::SensitivityParameter::EntryZScore;

    result.number_of_folds = 2;

    result.points.resize(2);

    for (std::size_t i = 0; i < 2; ++i) {
        auto& point =
            result.points[i];

        point.parameter =
            quant::research::SensitivityParameter::EntryZScore;

        point.parameter_value =
            0.5 +
            static_cast<double>(i);

        point.number_of_observed_folds = 2;

        point.mean_total_return = 0.08;
        point.standard_deviation_total_return = 0.01;

        point.mean_sharpe = 1.0;
        point.standard_deviation_sharpe = 0.1;

        point.mean_total_return_rank = 1.5;
        point.standard_deviation_total_return_rank = 0.5;

        point.mean_sharpe_rank = 1.5;
        point.standard_deviation_sharpe_rank = 0.5;
    }

    return result;
}

quant::research::BootstrapResult
make_bootstrap_result() {
    quant::research::BootstrapResult result{};

    result.observations = 100;
    result.number_of_resamples = 100;
    result.block_length = 5;

    result.observed_total_return = 0.10;
    result.observed_annualized_volatility = 0.10;
    result.observed_sharpe_ratio = 1.50;
    result.observed_maximum_drawdown = 0.05;
    result.observed_maximum_drawdown_pct = 0.05;

    result.total_return_distribution.resize(
        100,
        0.10
    );

    result.annualized_volatility_distribution.resize(
        100,
        0.10
    );

    result.sharpe_ratio_distribution.resize(
        100,
        1.0
    );

    result.maximum_drawdown_distribution.resize(
        100,
        0.05
    );

    result.maximum_drawdown_pct_distribution.resize(
        100,
        0.05
    );

    result.sharpe_ratio_interval =
        {
            0.50,
            2.00
        };

    return result;
}

quant::research::PlaceboResult
make_placebo_result() {
    quant::research::PlaceboResult result{};

    result.observed_total_return = 0.10;
    result.observed_sharpe_ratio = 1.50;

    result.number_of_placebos_requested = 100;
    result.number_of_successful_placebos = 100;

    result.placebo_total_returns.resize(
        100,
        0.01
    );

    result.placebo_sharpes.resize(
        100,
        0.20
    );

    result.empirical_p_value_total_return = 0.02;
    result.empirical_p_value_sharpe_ratio = 0.02;

    return result;
}

} // namespace

int main() {
    const auto experiment =
        make_experiment();

    const auto walk_forward =
        make_walk_forward_result();

    const auto performance =
        *walk_forward.aggregate_performance;

    const auto cost =
        make_cost_result();

    const auto parameter =
        make_parameter_result();

    const auto subperiod =
        make_subperiod_result();

    const auto walk_forward_parameter =
        make_walk_forward_parameter_result();

    const auto bootstrap =
        make_bootstrap_result();

    const auto placebo =
        make_placebo_result();

    const auto result =
        quant::research::assemble_research_experiment(
            experiment,
            walk_forward,
            performance,
            cost,
            parameter,
            subperiod,
            walk_forward_parameter,
            bootstrap,
            placebo
        );

    assert(
        result.experiment.experiment_id ==
        "pairs_001"
    );

    assert(
        result.experiment.description ==
        "Synthetic pairs research experiment"
    );

    assert(
        result.walk_forward.folds.size() == 1
    );

    assert(
        result.performance.total_return ==
        0.01
    );

    assert(
        result.bootstrap.number_of_resamples ==
        100
    );

    assert(
        result.placebo.number_of_successful_placebos ==
        100
    );

    assert(
        result.robustness.checks_total == 6
    );

    /*
        Every synthetic diagnostic satisfies the configured
        criteria, so the complete experiment must be Robust.
    */
    assert(
        result.robustness.status ==
        quant::research::RobustnessStatus::Robust
    );

    assert(
        result.robustness.checks_passed == 6
    );

    std::cout
        << "Research experiment tests passed!\n";

    return 0;
}