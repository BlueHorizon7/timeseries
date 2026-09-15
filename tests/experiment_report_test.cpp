#include "quant/research/experiment_report.hpp"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace {

quant::research::ResearchExperimentResult
make_result() {
    quant::research::ResearchExperimentResult result{};

    result.experiment.experiment_id =
        "report_test_001";

    result.experiment.description =
        "Deterministic report serialization test";

    result.experiment.periods_per_year =
        252.0;

    result.experiment.walk_forward_parameters
        .formation_size = 350;

    result.experiment.walk_forward_parameters
        .test_size = 50;

    result.performance.initial_equity =
        10000.0;

    result.performance.final_equity =
        11000.0;

    result.performance.total_pnl =
        1000.0;

    result.performance.total_return =
        0.10;

    result.performance.annualized_return =
        0.12;

    result.performance.annualized_volatility =
        0.15;

    result.performance.sharpe_ratio =
        0.80;

    result.performance.maximum_drawdown =
        0.05;

    result.performance.maximum_drawdown_pct =
        0.05;

    result.performance.number_of_bars =
        100;

    result.performance.number_of_position_changes =
        10;

    /*
        Robustness result.
    */
    result.robustness.status =
        quant::research::RobustnessStatus::Robust;

    result.robustness.cost_robust = true;
    result.robustness.parameter_robust = true;
    result.robustness.subperiod_robust = true;
    result.robustness.walk_forward_parameter_robust = true;
    result.robustness.bootstrap_supportive = true;
    result.robustness.placebo_supportive = true;

    result.robustness.checks_passed = 6;
    result.robustness.checks_total = 6;

    /*
        Cost sensitivity.
    */
    result.cost_sensitivity.break_even_found = true;

    result.cost_sensitivity.break_even_cost_multiplier =
        2.0;

    /*
        Parameter sensitivity.
    */
    result.parameter_sensitivity.parameter =
        quant::research::SensitivityParameter::EntryZScore;

    quant::research::ParameterSensitivityPoint
        parameter_point{};

    parameter_point.parameter =
        quant::research::SensitivityParameter::EntryZScore;

    parameter_point.parameter_value =
        1.5;

    parameter_point.performance =
        result.performance;

    result.parameter_sensitivity.points.push_back(
        parameter_point
    );

    /*
        Subperiod stability.
    */
    quant::research::SubperiodPerformance
        subperiod{};

    subperiod.begin_bar =
        0;

    subperiod.end_bar =
        50;

    subperiod.performance =
        result.performance;

    result.subperiod_stability.periods.push_back(
        subperiod
    );

    result.subperiod_stability.best_total_return =
        0.10;

    result.subperiod_stability.worst_sharpe =
        0.50;

    result.subperiod_stability.best_sharpe =
        1.20;

    result.subperiod_stability.profitable_periods =
        1;

    result.subperiod_stability.losing_periods =
        0;

    /*
        Walk-forward parameter stability.
    */
    result.walk_forward_parameter_stability.parameter =
        quant::research::SensitivityParameter::EntryZScore;

    result.walk_forward_parameter_stability.number_of_folds =
        2;

    quant::research::WalkForwardParameterStabilityPoint
        walk_forward_point{};

    walk_forward_point.parameter =
        quant::research::SensitivityParameter::EntryZScore;

    walk_forward_point.parameter_value =
        1.5;

    walk_forward_point.fold_total_returns =
        {0.05, 0.08};

    walk_forward_point.fold_sharpes =
        {0.8, 1.1};

    walk_forward_point.mean_total_return =
        0.065;

    walk_forward_point.standard_deviation_total_return =
        0.015;

    walk_forward_point.mean_sharpe =
        0.95;

    walk_forward_point.standard_deviation_sharpe =
        0.15;

    walk_forward_point.mean_total_return_rank =
        1.0;

    walk_forward_point.standard_deviation_total_return_rank =
        0.0;

    walk_forward_point.mean_sharpe_rank =
        1.0;

    walk_forward_point.standard_deviation_sharpe_rank =
        0.0;

    walk_forward_point.number_of_observed_folds =
        2;

    result.walk_forward_parameter_stability.points.push_back(
        walk_forward_point
    );

    /*
        Bootstrap.
    */
    result.bootstrap.observations =
        100;

    result.bootstrap.number_of_resamples =
        10;

    result.bootstrap.block_length =
        5;

    result.bootstrap.observed_total_return =
        0.10;

    result.bootstrap.observed_annualized_volatility =
        0.15;

    result.bootstrap.observed_sharpe_ratio =
        0.80;

    result.bootstrap.observed_maximum_drawdown =
        0.05;

    result.bootstrap.observed_maximum_drawdown_pct =
        0.05;

    result.bootstrap.total_return_distribution =
        {0.05, 0.06, 0.07};

    result.bootstrap.annualized_volatility_distribution =
        {0.12, 0.14, 0.16};

    result.bootstrap.sharpe_ratio_distribution =
        {0.5, 0.8, 1.0};

    result.bootstrap.maximum_drawdown_distribution =
        {0.03, 0.04, 0.05};

    result.bootstrap.maximum_drawdown_pct_distribution =
        {0.03, 0.04, 0.05};

    result.bootstrap.total_return_interval =
        {0.05, 0.09};

    result.bootstrap.annualized_volatility_interval =
        {0.12, 0.17};

    result.bootstrap.sharpe_ratio_interval =
        {0.40, 1.10};

    result.bootstrap.maximum_drawdown_interval =
        {0.02, 0.07};

    result.bootstrap.maximum_drawdown_pct_interval =
        {0.02, 0.07};

    /*
        Placebo.
    */
    result.placebo.observed_total_return =
        0.10;

    result.placebo.observed_sharpe_ratio =
        0.80;

    result.placebo.number_of_placebos_requested =
        10;

    result.placebo.number_of_successful_placebos =
        10;

    result.placebo.placebo_total_returns =
        {0.00, 0.01, 0.02};

    result.placebo.placebo_sharpes =
        {0.00, 0.10, 0.20};

    result.placebo.empirical_p_value_total_return =
        0.05;

    result.placebo.empirical_p_value_sharpe_ratio =
        0.04;

    return result;
}

} // namespace

int main() {
    const auto result =
        make_result();

    /*
        --------------------------------------------------------
        Basic JSON generation.
        --------------------------------------------------------
    */
    const std::string json =
        quant::research::research_experiment_to_json(
            result
        );

    assert(!json.empty());

    /*
        Verify important structural fields rather than depending
        on an exact binary floating-point text representation.
    */
    assert(
        json.find(
            "\"experiment_id\": \"report_test_001\""
        ) != std::string::npos
    );

    assert(
        json.find(
            "\"description\": "
            "\"Deterministic report serialization test\""
        ) != std::string::npos
    );

    assert(
        json.find(
            "\"total_return\":"
        ) != std::string::npos
    );

    assert(
        json.find(
            "\"sharpe_ratio\":"
        ) != std::string::npos
    );

    assert(
        json.find(
            "\"status\": \"Robust\""
        ) != std::string::npos
    );

    assert(
        json.find(
            "\"EntryZScore\""
        ) != std::string::npos
    );

    assert(
        json.find(
            "\"empirical_p_value_sharpe_ratio\":"
        ) != std::string::npos
    );

    /*
        --------------------------------------------------------
        Determinism.
        --------------------------------------------------------
    */
    const std::string json_again =
        quant::research::research_experiment_to_json(
            result
        );

    assert(
        json == json_again
    );

    /*
        --------------------------------------------------------
        File serialization.

        Use a simple working-directory file rather than
        std::filesystem::temp_directory_path().
        --------------------------------------------------------
    */
    const std::string file_path =
        "quantlab_experiment_report_test.json";

    quant::research::write_research_experiment_json(
        result,
        file_path
    );

    std::ifstream input(
        file_path,
        std::ios::binary
    );

    assert(input.good());

    const std::string file_contents{
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };

    input.close();

    assert(
        file_contents == json
    );

    const int remove_result =
        std::remove(
            file_path.c_str()
        );

    assert(
        remove_result == 0
    );

    /*
        --------------------------------------------------------
        Invalid empty output path.
        --------------------------------------------------------
    */
    {
        bool threw = false;

        try {
            quant::research::write_research_experiment_json(
                result,
                ""
            );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        --------------------------------------------------------
        Robustness enum serialization.
        --------------------------------------------------------
    */
    assert(
        quant::research::robustness_status_to_string(
            quant::research::RobustnessStatus::Robust
        ) == "Robust"
    );

    assert(
        quant::research::robustness_status_to_string(
            quant::research::RobustnessStatus::Fragile
        ) == "Fragile"
    );

    assert(
        quant::research::robustness_status_to_string(
            quant::research::RobustnessStatus::Inconclusive
        ) == "Inconclusive"
    );

    assert(
        quant::research::robustness_status_to_string(
            quant::research::RobustnessStatus::Failed
        ) == "Failed"
    );

    /*
        --------------------------------------------------------
        Sensitivity parameter enum serialization.
        --------------------------------------------------------
    */
    assert(
        quant::research::sensitivity_parameter_to_string(
            quant::research::SensitivityParameter::
                HedgeRatioWindow
        ) == "HedgeRatioWindow"
    );

    assert(
        quant::research::sensitivity_parameter_to_string(
            quant::research::SensitivityParameter::
                ZScoreWindow
        ) == "ZScoreWindow"
    );

    assert(
        quant::research::sensitivity_parameter_to_string(
            quant::research::SensitivityParameter::
                EntryZScore
        ) == "EntryZScore"
    );

    assert(
        quant::research::sensitivity_parameter_to_string(
            quant::research::SensitivityParameter::
                ExitZScore
        ) == "ExitZScore"
    );

    std::cout
        << "Experiment report tests passed!\n";

    return 0;
}