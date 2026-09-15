#include "quant/research/robustness_report.hpp"

#include <cassert>
#include <iostream>

namespace {

quant::research::CostSensitivityResult
make_cost_result(bool robust) {
    quant::research::CostSensitivityResult result{};

    result.points.resize(3);

    result.points[0].cost_multiplier = 0.5;
    result.points[1].cost_multiplier = 1.0;
    result.points[2].cost_multiplier = 2.0;

    for (auto& point : result.points) {
        point.performance.total_return =
            robust
                ? 0.10
                : -0.05;
    }

    result.break_even_found = true;

    result.break_even_cost_multiplier =
        robust
            ? 2.0
            : 0.5;

    return result;
}

quant::research::ParameterSensitivityResult
make_parameter_result(bool robust) {
    quant::research::ParameterSensitivityResult result{};

    result.parameter =
        quant::research::SensitivityParameter::EntryZScore;

    result.points.resize(4);

    for (std::size_t i = 0;
         i < result.points.size();
         ++i) {

        result.points[i].parameter =
            quant::research::SensitivityParameter::EntryZScore;

        result.points[i].parameter_value =
            0.5 +
            0.5 * static_cast<double>(i);

        /*
            Robust case:
                all parameter values are profitable.

            Failure case:
                no parameter values are profitable.
        */
        result.points[i].performance.total_return =
            robust
                ? 0.10
                : -0.05;
    }

    return result;
}

quant::research::SubperiodStabilityResult
make_subperiod_result(bool robust) {
    quant::research::SubperiodStabilityResult result{};

    result.periods.resize(4);

    result.profitable_periods =
        robust
            ? 3
            : 0;

    result.losing_periods =
        4 - result.profitable_periods;

    result.best_total_return = 0.20;
    result.worst_sharpe = -0.50;
    result.best_sharpe = 1.80;

    for (std::size_t i = 0;
         i < result.periods.size();
         ++i) {

        result.periods[i].begin_bar =
            i * 10;

        result.periods[i].end_bar =
            (i + 1) * 10;

        result.periods[i]
            .performance.total_return =
            robust
                ? (i < 3 ? 0.10 : -0.05)
                : -0.05;
    }

    return result;
}

quant::research::WalkForwardParameterStabilityResult
make_walk_forward_result(bool robust) {
    quant::research::WalkForwardParameterStabilityResult result{};

    result.parameter =
        quant::research::SensitivityParameter::EntryZScore;

    result.number_of_folds = 3;

    result.points.resize(4);

    for (std::size_t i = 0;
         i < result.points.size();
         ++i) {

        auto& point =
            result.points[i];

        point.parameter =
            quant::research::SensitivityParameter::EntryZScore;

        point.parameter_value =
            0.5 +
            0.5 * static_cast<double>(i);

        point.number_of_observed_folds = 3;

        /*
            Robust case:
                every parameter value has positive
                mean OOS return.

            Failure case:
                every parameter value has negative
                mean OOS return.
        */
        point.mean_total_return =
            robust
                ? 0.08
                : -0.03;

        point.standard_deviation_total_return =
            0.02;

        point.mean_sharpe =
            robust
                ? 1.0
                : -0.50;

        point.standard_deviation_sharpe =
            0.10;

        point.mean_total_return_rank =
            1.5;

        point.standard_deviation_total_return_rank =
            0.5;

        point.mean_sharpe_rank =
            1.5;

        point.standard_deviation_sharpe_rank =
            0.5;
    }

    return result;
}

quant::research::BootstrapResult
make_bootstrap_result(bool robust) {
    quant::research::BootstrapResult result{};

    result.observations = 100;
    result.number_of_resamples = 500;
    result.block_length = 5;

    result.observed_total_return =
        0.20;

    result.observed_annualized_volatility =
        0.15;

    result.observed_sharpe_ratio =
        1.50;

    result.observed_maximum_drawdown =
        0.08;

    result.observed_maximum_drawdown_pct =
        0.08;

    result.sharpe_ratio_distribution.resize(
        500,
        robust
            ? 1.0
            : -0.50
    );

    /*
        Robust:
            lower confidence bound > 0.

        Failure:
            lower confidence bound < 0.
    */
    result.sharpe_ratio_interval =
        robust
            ? quant::research::BootstrapInterval{
                  0.25,
                  2.00
              }
            : quant::research::BootstrapInterval{
                  -0.25,
                  1.00
              };

    return result;
}

quant::research::PlaceboResult
make_placebo_result(bool robust) {
    quant::research::PlaceboResult result{};

    result.observed_total_return =
        0.20;

    result.observed_sharpe_ratio =
        1.50;

    result.number_of_placebos_requested =
        500;

    result.number_of_successful_placebos =
        500;

    result.placebo_total_returns.resize(
        500,
        robust
            ? 0.01
            : 0.10
    );

    result.placebo_sharpes.resize(
        500,
        robust
            ? 0.20
            : 1.20
    );

    result.empirical_p_value_total_return =
        robust
            ? 0.02
            : 0.50;

    result.empirical_p_value_sharpe_ratio =
        robust
            ? 0.02
            : 0.50;

    return result;
}

quant::research::RobustnessCriteria
make_criteria() {
    quant::research::RobustnessCriteria criteria{};

    criteria.minimum_break_even_cost_multiplier =
        1.0;

    criteria.minimum_profitable_subperiod_fraction =
        0.50;

    criteria.minimum_profitable_parameter_fraction =
        0.50;

    criteria.minimum_profitable_walk_forward_parameter_fraction =
        0.50;

    criteria.minimum_bootstrap_sharpe_lower_bound =
        0.0;

    criteria.maximum_placebo_sharpe_p_value =
        0.05;

    criteria.minimum_successful_placebos =
        100;

    criteria.require_all_diagnostics =
        true;

    return criteria;
}

} // namespace

int main() {
    const auto criteria =
        make_criteria();

    /*
        --------------------------------------------------------
        Case 1: every robustness dimension passes.
        Expected status: Robust.
        --------------------------------------------------------
    */
    {
        const auto result =
            quant::research::assess_robustness(
                make_cost_result(true),
                make_parameter_result(true),
                make_subperiod_result(true),
                make_walk_forward_result(true),
                make_bootstrap_result(true),
                make_placebo_result(true),
                criteria
            );

        assert(
            result.status ==
            quant::research::RobustnessStatus::Robust
        );

        assert(result.cost_robust);
        assert(result.parameter_robust);
        assert(result.subperiod_robust);
        assert(result.walk_forward_parameter_robust);
        assert(result.bootstrap_supportive);
        assert(result.placebo_supportive);

        assert(result.checks_passed == 6);
        assert(result.checks_total == 6);

        assert(result.warnings.empty());
    }

    /*
        --------------------------------------------------------
        Case 2: exactly one robustness dimension fails.
        Expected status: Fragile.
        --------------------------------------------------------
    */
    {
        const auto result =
            quant::research::assess_robustness(
                make_cost_result(false),
                make_parameter_result(true),
                make_subperiod_result(true),
                make_walk_forward_result(true),
                make_bootstrap_result(true),
                make_placebo_result(true),
                criteria
            );

        assert(
            result.status ==
            quant::research::RobustnessStatus::Fragile
        );

        assert(!result.cost_robust);

        assert(result.parameter_robust);
        assert(result.subperiod_robust);
        assert(result.walk_forward_parameter_robust);
        assert(result.bootstrap_supportive);
        assert(result.placebo_supportive);

        assert(result.checks_passed == 5);
        assert(result.checks_total == 6);

        assert(result.warnings.size() == 1);
    }

    /*
        --------------------------------------------------------
        Case 3: every robustness dimension fails.
        Expected status: Failed.
        --------------------------------------------------------
    */
    {
        const auto result =
            quant::research::assess_robustness(
                make_cost_result(false),
                make_parameter_result(false),
                make_subperiod_result(false),
                make_walk_forward_result(false),
                make_bootstrap_result(false),
                make_placebo_result(false),
                criteria
            );

        assert(
            result.status ==
            quant::research::RobustnessStatus::Failed
        );

        assert(!result.cost_robust);
        assert(!result.parameter_robust);
        assert(!result.subperiod_robust);
        assert(!result.walk_forward_parameter_robust);
        assert(!result.bootstrap_supportive);
        assert(!result.placebo_supportive);

        assert(result.checks_passed == 0);
        assert(result.checks_total == 6);

        assert(result.warnings.size() == 6);
    }

    /*
        --------------------------------------------------------
        Case 4: invalid policy.
        --------------------------------------------------------
    */
    {
        auto invalid_criteria =
            criteria;

        invalid_criteria
            .minimum_profitable_subperiod_fraction =
            1.5;

        bool threw = false;

        try {
            (void)
                quant::research::assess_robustness(
                    make_cost_result(true),
                    make_parameter_result(true),
                    make_subperiod_result(true),
                    make_walk_forward_result(true),
                    make_bootstrap_result(true),
                    make_placebo_result(true),
                    invalid_criteria
                );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        --------------------------------------------------------
        Case 5: invalid parameter profitability threshold.
        --------------------------------------------------------
    */
    {
        auto invalid_criteria =
            criteria;

        invalid_criteria
            .minimum_profitable_parameter_fraction =
            -0.1;

        bool threw = false;

        try {
            (void)
                quant::research::assess_robustness(
                    make_cost_result(true),
                    make_parameter_result(true),
                    make_subperiod_result(true),
                    make_walk_forward_result(true),
                    make_bootstrap_result(true),
                    make_placebo_result(true),
                    invalid_criteria
                );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        --------------------------------------------------------
        Case 6: invalid placebo threshold.
        --------------------------------------------------------
    */
    {
        auto invalid_criteria =
            criteria;

        invalid_criteria
            .maximum_placebo_sharpe_p_value =
            0.0;

        bool threw = false;

        try {
            (void)
                quant::research::assess_robustness(
                    make_cost_result(true),
                    make_parameter_result(true),
                    make_subperiod_result(true),
                    make_walk_forward_result(true),
                    make_bootstrap_result(true),
                    make_placebo_result(true),
                    invalid_criteria
                );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        --------------------------------------------------------
        Case 7: invalid bootstrap threshold.
        --------------------------------------------------------
    */
    {
        auto invalid_criteria =
            criteria;

        invalid_criteria
            .minimum_bootstrap_sharpe_lower_bound =
            1.0 / 0.0;

        bool threw = false;

        try {
            (void)
                quant::research::assess_robustness(
                    make_cost_result(true),
                    make_parameter_result(true),
                    make_subperiod_result(true),
                    make_walk_forward_result(true),
                    make_bootstrap_result(true),
                    make_placebo_result(true),
                    invalid_criteria
                );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    std::cout
        << "Robustness report tests passed!\n";

    return 0;
}