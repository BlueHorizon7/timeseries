#include "quant/research/experiment_report.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

namespace quant::research {

namespace {

void append_indent(
    std::ostringstream& output,
    int level
) {
    output << std::string(
        static_cast<std::size_t>(level) * 2,
        ' '
    );
}

std::string escape_json(
    const std::string& value
) {
    std::ostringstream output;

    for (const char character : value) {
        switch (character) {
        case '"':
            output << "\\\"";
            break;

        case '\\':
            output << "\\\\";
            break;

        case '\b':
            output << "\\b";
            break;

        case '\f':
            output << "\\f";
            break;

        case '\n':
            output << "\\n";
            break;

        case '\r':
            output << "\\r";
            break;

        case '\t':
            output << "\\t";
            break;

        default:
            output << character;
            break;
        }
    }

    return output.str();
}

void append_string(
    std::ostringstream& output,
    const std::string& value
) {
    output << '"'
           << escape_json(value)
           << '"';
}

void append_double(
    std::ostringstream& output,
    double value
) {
    output << std::setprecision(17)
           << value;
}

void append_performance(
    std::ostringstream& output,
    const quant::risk::PerformanceMetrics& performance,
    int level
) {
    append_indent(output, level);
    output << "{\n";

    append_indent(output, level + 1);
    output << "\"initial_equity\": ";
    append_double(
        output,
        performance.initial_equity
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"final_equity\": ";
    append_double(
        output,
        performance.final_equity
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"total_pnl\": ";
    append_double(
        output,
        performance.total_pnl
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"total_return\": ";
    append_double(
        output,
        performance.total_return
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"annualized_return\": ";
    append_double(
        output,
        performance.annualized_return
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"annualized_volatility\": ";
    append_double(
        output,
        performance.annualized_volatility
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"sharpe_ratio\": ";
    append_double(
        output,
        performance.sharpe_ratio
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"maximum_drawdown\": ";
    append_double(
        output,
        performance.maximum_drawdown
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"maximum_drawdown_pct\": ";
    append_double(
        output,
        performance.maximum_drawdown_pct
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"number_of_bars\": "
           << performance.number_of_bars
           << ",\n";

    append_indent(output, level + 1);
    output << "\"number_of_position_changes\": "
           << performance.number_of_position_changes
           << '\n';

    append_indent(output, level);
    output << "}";
}

void append_double_vector(
    std::ostringstream& output,
    const std::vector<double>& values,
    int level
) {
    output << "[\n";

    for (std::size_t i = 0; i < values.size(); ++i) {
        append_indent(output, level + 1);
        append_double(
            output,
            values[i]
        );

        if (i + 1 < values.size()) {
            output << ',';
        }

        output << '\n';
    }

    append_indent(output, level);
    output << "]";
}

void append_cost_sensitivity(
    std::ostringstream& output,
    const CostSensitivityResult& result,
    int level
) {
    append_indent(output, level);
    output << "{\n";

    append_indent(output, level + 1);
    output << "\"break_even_found\": "
           << (result.break_even_found ? "true" : "false")
           << ",\n";

    append_indent(output, level + 1);
    output << "\"break_even_cost_multiplier\": ";
    append_double(
        output,
        result.break_even_cost_multiplier
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"points\": [\n";

    for (std::size_t i = 0;
         i < result.points.size();
         ++i) {

        const auto& point =
            result.points[i];

        append_indent(output, level + 2);
        output << "{\n";

        append_indent(output, level + 3);
        output << "\"cost_multiplier\": ";
        append_double(
            output,
            point.cost_multiplier
        );
        output << ",\n";

        append_indent(output, level + 3);
        output << "\"performance\": ";
        append_performance(
            output,
            point.performance,
            level + 3
        );
        output << ",\n";

        append_indent(output, level + 3);
        output << "\"total_transaction_cost\": ";
        append_double(
            output,
            point.total_transaction_cost
        );
        output << ",\n";

        append_indent(output, level + 3);
        output << "\"total_execution_cost\": ";
        append_double(
            output,
            point.total_execution_cost
        );
        output << '\n';

        append_indent(output, level + 2);
        output << "}";

        if (i + 1 < result.points.size()) {
            output << ',';
        }

        output << '\n';
    }

    append_indent(output, level + 1);
    output << "]\n";

    append_indent(output, level);
    output << "}";
}

void append_parameter_sensitivity(
    std::ostringstream& output,
    const ParameterSensitivityResult& result,
    int level
) {
    append_indent(output, level);
    output << "{\n";

    append_indent(output, level + 1);
    output << "\"parameter\": ";
    append_string(
        output,
        sensitivity_parameter_to_string(
            result.parameter
        )
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"points\": [\n";

    for (std::size_t i = 0;
         i < result.points.size();
         ++i) {

        const auto& point =
            result.points[i];

        append_indent(output, level + 2);
        output << "{\n";

        append_indent(output, level + 3);
        output << "\"parameter\": ";
        append_string(
            output,
            sensitivity_parameter_to_string(
                point.parameter
            )
        );
        output << ",\n";

        append_indent(output, level + 3);
        output << "\"parameter_value\": ";
        append_double(
            output,
            point.parameter_value
        );
        output << ",\n";

        append_indent(output, level + 3);
        output << "\"performance\": ";
        append_performance(
            output,
            point.performance,
            level + 3
        );
        output << '\n';

        append_indent(output, level + 2);
        output << "}";

        if (i + 1 < result.points.size()) {
            output << ',';
        }

        output << '\n';
    }

    append_indent(output, level + 1);
    output << "]\n";

    append_indent(output, level);
    output << "}";
}

void append_subperiod_stability(
    std::ostringstream& output,
    const SubperiodStabilityResult& result,
    int level
) {
    append_indent(output, level);
    output << "{\n";

    append_indent(output, level + 1);
    output << "\"best_total_return\": ";
    append_double(
        output,
        result.best_total_return
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"worst_sharpe\": ";
    append_double(
        output,
        result.worst_sharpe
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"best_sharpe\": ";
    append_double(
        output,
        result.best_sharpe
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"profitable_periods\": "
           << result.profitable_periods
           << ",\n";

    append_indent(output, level + 1);
    output << "\"losing_periods\": "
           << result.losing_periods
           << ",\n";

    append_indent(output, level + 1);
    output << "\"periods\": [\n";

    for (std::size_t i = 0;
         i < result.periods.size();
         ++i) {

        const auto& period =
            result.periods[i];

        append_indent(output, level + 2);
        output << "{\n";

        append_indent(output, level + 3);
        output << "\"begin_bar\": "
               << period.begin_bar
               << ",\n";

        append_indent(output, level + 3);
        output << "\"end_bar\": "
               << period.end_bar
               << ",\n";

        append_indent(output, level + 3);
        output << "\"performance\": ";
        append_performance(
            output,
            period.performance,
            level + 3
        );
        output << '\n';

        append_indent(output, level + 2);
        output << "}";

        if (i + 1 < result.periods.size()) {
            output << ',';
        }

        output << '\n';
    }

    append_indent(output, level + 1);
    output << "]\n";

    append_indent(output, level);
    output << "}";
}

void append_walk_forward_parameter_stability(
    std::ostringstream& output,
    const WalkForwardParameterStabilityResult& result,
    int level
) {
    append_indent(output, level);
    output << "{\n";

    append_indent(output, level + 1);
    output << "\"parameter\": ";
    append_string(
        output,
        sensitivity_parameter_to_string(
            result.parameter
        )
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"number_of_folds\": "
           << result.number_of_folds
           << ",\n";

    append_indent(output, level + 1);
    output << "\"points\": [\n";

    for (std::size_t i = 0;
         i < result.points.size();
         ++i) {

        const auto& point =
            result.points[i];

        append_indent(output, level + 2);
        output << "{\n";

        append_indent(output, level + 3);
        output << "\"parameter\": ";
        append_string(
            output,
            sensitivity_parameter_to_string(
                point.parameter
            )
        );
        output << ",\n";

        append_indent(output, level + 3);
        output << "\"parameter_value\": ";
        append_double(
            output,
            point.parameter_value
        );
        output << ",\n";

        append_indent(output, level + 3);
        output << "\"fold_total_returns\": ";
        append_double_vector(
            output,
            point.fold_total_returns,
            level + 3
        );
        output << ",\n";

        append_indent(output, level + 3);
        output << "\"fold_sharpes\": ";
        append_double_vector(
            output,
            point.fold_sharpes,
            level + 3
        );
        output << ",\n";

        append_indent(output, level + 3);
        output << "\"mean_total_return\": ";
        append_double(
            output,
            point.mean_total_return
        );
        output << ",\n";

        append_indent(output, level + 3);
        output << "\"standard_deviation_total_return\": ";
        append_double(
            output,
            point.standard_deviation_total_return
        );
        output << ",\n";

        append_indent(output, level + 3);
        output << "\"mean_sharpe\": ";
        append_double(
            output,
            point.mean_sharpe
        );
        output << ",\n";

        append_indent(output, level + 3);
        output << "\"standard_deviation_sharpe\": ";
        append_double(
            output,
            point.standard_deviation_sharpe
        );
        output << ",\n";

        append_indent(output, level + 3);
        output << "\"mean_total_return_rank\": ";
        append_double(
            output,
            point.mean_total_return_rank
        );
        output << ",\n";

        append_indent(output, level + 3);
        output << "\"standard_deviation_total_return_rank\": ";
        append_double(
            output,
            point.standard_deviation_total_return_rank
        );
        output << ",\n";

        append_indent(output, level + 3);
        output << "\"mean_sharpe_rank\": ";
        append_double(
            output,
            point.mean_sharpe_rank
        );
        output << ",\n";

        append_indent(output, level + 3);
        output << "\"standard_deviation_sharpe_rank\": ";
        append_double(
            output,
            point.standard_deviation_sharpe_rank
        );
        output << ",\n";

        append_indent(output, level + 3);
        output << "\"number_of_observed_folds\": "
               << point.number_of_observed_folds
               << '\n';

        append_indent(output, level + 2);
        output << "}";

        if (i + 1 < result.points.size()) {
            output << ',';
        }

        output << '\n';
    }

    append_indent(output, level + 1);
    output << "]\n";

    append_indent(output, level);
    output << "}";
}

void append_bootstrap(
    std::ostringstream& output,
    const BootstrapResult& result,
    int level
) {
    append_indent(output, level);
    output << "{\n";

    append_indent(output, level + 1);
    output << "\"observations\": "
           << result.observations
           << ",\n";

    append_indent(output, level + 1);
    output << "\"number_of_resamples\": "
           << result.number_of_resamples
           << ",\n";

    append_indent(output, level + 1);
    output << "\"block_length\": "
           << result.block_length
           << ",\n";

    append_indent(output, level + 1);
    output << "\"observed_total_return\": ";
    append_double(
        output,
        result.observed_total_return
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"observed_annualized_volatility\": ";
    append_double(
        output,
        result.observed_annualized_volatility
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"observed_sharpe_ratio\": ";
    append_double(
        output,
        result.observed_sharpe_ratio
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"observed_maximum_drawdown\": ";
    append_double(
        output,
        result.observed_maximum_drawdown
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"observed_maximum_drawdown_pct\": ";
    append_double(
        output,
        result.observed_maximum_drawdown_pct
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"total_return_distribution\": ";
    append_double_vector(
        output,
        result.total_return_distribution,
        level + 1
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"annualized_volatility_distribution\": ";
    append_double_vector(
        output,
        result.annualized_volatility_distribution,
        level + 1
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"sharpe_ratio_distribution\": ";
    append_double_vector(
        output,
        result.sharpe_ratio_distribution,
        level + 1
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"maximum_drawdown_distribution\": ";
    append_double_vector(
        output,
        result.maximum_drawdown_distribution,
        level + 1
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"maximum_drawdown_pct_distribution\": ";
    append_double_vector(
        output,
        result.maximum_drawdown_pct_distribution,
        level + 1
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"total_return_interval\": {\n";

    append_indent(output, level + 2);
    output << "\"lower\": ";
    append_double(
        output,
        result.total_return_interval.lower
    );
    output << ",\n";

    append_indent(output, level + 2);
    output << "\"upper\": ";
    append_double(
        output,
        result.total_return_interval.upper
    );
    output << "\n";

    append_indent(output, level + 1);
    output << "},\n";

    append_indent(output, level + 1);
    output << "\"annualized_volatility_interval\": {\n";

    append_indent(output, level + 2);
    output << "\"lower\": ";
    append_double(
        output,
        result.annualized_volatility_interval.lower
    );
    output << ",\n";

    append_indent(output, level + 2);
    output << "\"upper\": ";
    append_double(
        output,
        result.annualized_volatility_interval.upper
    );
    output << "\n";

    append_indent(output, level + 1);
    output << "},\n";

    append_indent(output, level + 1);
    output << "\"sharpe_ratio_interval\": {\n";

    append_indent(output, level + 2);
    output << "\"lower\": ";
    append_double(
        output,
        result.sharpe_ratio_interval.lower
    );
    output << ",\n";

    append_indent(output, level + 2);
    output << "\"upper\": ";
    append_double(
        output,
        result.sharpe_ratio_interval.upper
    );
    output << "\n";

    append_indent(output, level + 1);
    output << "},\n";

    append_indent(output, level + 1);
    output << "\"maximum_drawdown_interval\": {\n";

    append_indent(output, level + 2);
    output << "\"lower\": ";
    append_double(
        output,
        result.maximum_drawdown_interval.lower
    );
    output << ",\n";

    append_indent(output, level + 2);
    output << "\"upper\": ";
    append_double(
        output,
        result.maximum_drawdown_interval.upper
    );
    output << "\n";

    append_indent(output, level + 1);
    output << "},\n";

    append_indent(output, level + 1);
    output << "\"maximum_drawdown_pct_interval\": {\n";

    append_indent(output, level + 2);
    output << "\"lower\": ";
    append_double(
        output,
        result.maximum_drawdown_pct_interval.lower
    );
    output << ",\n";

    append_indent(output, level + 2);
    output << "\"upper\": ";
    append_double(
        output,
        result.maximum_drawdown_pct_interval.upper
    );
    output << "\n";

    append_indent(output, level + 1);
    output << "}\n";

    append_indent(output, level);
    output << "}";
}

void append_placebo(
    std::ostringstream& output,
    const PlaceboResult& result,
    int level
) {
    append_indent(output, level);
    output << "{\n";

    append_indent(output, level + 1);
    output << "\"observed_total_return\": ";
    append_double(
        output,
        result.observed_total_return
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"observed_sharpe_ratio\": ";
    append_double(
        output,
        result.observed_sharpe_ratio
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"number_of_placebos_requested\": "
           << result.number_of_placebos_requested
           << ",\n";

    append_indent(output, level + 1);
    output << "\"number_of_successful_placebos\": "
           << result.number_of_successful_placebos
           << ",\n";

    append_indent(output, level + 1);
    output << "\"empirical_p_value_total_return\": ";
    append_double(
        output,
        result.empirical_p_value_total_return
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"empirical_p_value_sharpe_ratio\": ";
    append_double(
        output,
        result.empirical_p_value_sharpe_ratio
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"placebo_total_returns\": ";
    append_double_vector(
        output,
        result.placebo_total_returns,
        level + 1
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"placebo_sharpes\": ";
    append_double_vector(
        output,
        result.placebo_sharpes,
        level + 1
    );
    output << '\n';

    append_indent(output, level);
    output << "}";
}

void append_robustness(
    std::ostringstream& output,
    const RobustnessAssessment& result,
    int level
) {
    append_indent(output, level);
    output << "{\n";

    append_indent(output, level + 1);
    output << "\"status\": ";
    append_string(
        output,
        robustness_status_to_string(
            result.status
        )
    );
    output << ",\n";

    append_indent(output, level + 1);
    output << "\"cost_robust\": "
           << (result.cost_robust ? "true" : "false")
           << ",\n";

    append_indent(output, level + 1);
    output << "\"parameter_robust\": "
           << (result.parameter_robust ? "true" : "false")
           << ",\n";

    append_indent(output, level + 1);
    output << "\"subperiod_robust\": "
           << (result.subperiod_robust ? "true" : "false")
           << ",\n";

    append_indent(output, level + 1);
    output << "\"walk_forward_parameter_robust\": "
           << (
               result.walk_forward_parameter_robust
                   ? "true"
                   : "false"
           )
           << ",\n";

    append_indent(output, level + 1);
    output << "\"bootstrap_supportive\": "
           << (
               result.bootstrap_supportive
                   ? "true"
                   : "false"
           )
           << ",\n";

    append_indent(output, level + 1);
    output << "\"placebo_supportive\": "
           << (
               result.placebo_supportive
                   ? "true"
                   : "false"
           )
           << ",\n";

    append_indent(output, level + 1);
    output << "\"checks_passed\": "
           << result.checks_passed
           << ",\n";

    append_indent(output, level + 1);
    output << "\"checks_total\": "
           << result.checks_total
           << ",\n";

    append_indent(output, level + 1);
    output << "\"warnings\": [\n";

    for (std::size_t i = 0;
         i < result.warnings.size();
         ++i) {

        append_indent(output, level + 2);

        append_string(
            output,
            result.warnings[i]
        );

        if (i + 1 < result.warnings.size()) {
            output << ',';
        }

        output << '\n';
    }

    append_indent(output, level + 1);
    output << "]\n";

    append_indent(output, level);
    output << "}";
}

} // namespace

std::string robustness_status_to_string(
    RobustnessStatus status
) {
    switch (status) {
    case RobustnessStatus::Robust:
        return "Robust";

    case RobustnessStatus::Fragile:
        return "Fragile";

    case RobustnessStatus::Inconclusive:
        return "Inconclusive";

    case RobustnessStatus::Failed:
        return "Failed";
    }

    throw std::invalid_argument(
        "Unknown robustness status"
    );
}

std::string sensitivity_parameter_to_string(
    SensitivityParameter parameter
) {
    switch (parameter) {
    case SensitivityParameter::HedgeRatioWindow:
        return "HedgeRatioWindow";

    case SensitivityParameter::ZScoreWindow:
        return "ZScoreWindow";

    case SensitivityParameter::EntryZScore:
        return "EntryZScore";

    case SensitivityParameter::ExitZScore:
        return "ExitZScore";
    }

    throw std::invalid_argument(
        "Unknown sensitivity parameter"
    );
}

std::string research_experiment_to_json(
    const ResearchExperimentResult& result
) {
    std::ostringstream output;

    output << std::setprecision(17);
    output << "{\n";

    output << "  \"experiment\": {\n";

    output << "    \"experiment_id\": ";
    append_string(
        output,
        result.experiment.experiment_id
    );
    output << ",\n";

    output << "    \"description\": ";
    append_string(
        output,
        result.experiment.description
    );
    output << ",\n";

    output << "    \"periods_per_year\": ";
    append_double(
        output,
        result.experiment.periods_per_year
    );
    output << ",\n";

    output << "    \"walk_forward\": {\n";

    output << "      \"formation_size\": "
           << result.experiment.walk_forward_parameters
                  .formation_size
           << ",\n";

    output << "      \"test_size\": "
           << result.experiment.walk_forward_parameters
                  .test_size
           << "\n";

    output << "    }\n";
    output << "  },\n";

    output << "  \"performance\": ";
    append_performance(
        output,
        result.performance,
        1
    );
    output << ",\n";

    output << "  \"cost_sensitivity\": ";
    append_cost_sensitivity(
        output,
        result.cost_sensitivity,
        1
    );
    output << ",\n";

    output << "  \"parameter_sensitivity\": ";
    append_parameter_sensitivity(
        output,
        result.parameter_sensitivity,
        1
    );
    output << ",\n";

    output << "  \"subperiod_stability\": ";
    append_subperiod_stability(
        output,
        result.subperiod_stability,
        1
    );
    output << ",\n";

    output << "  \"walk_forward_parameter_stability\": ";
    append_walk_forward_parameter_stability(
        output,
        result.walk_forward_parameter_stability,
        1
    );
    output << ",\n";

    output << "  \"bootstrap\": ";
    append_bootstrap(
        output,
        result.bootstrap,
        1
    );
    output << ",\n";

    output << "  \"placebo\": ";
    append_placebo(
        output,
        result.placebo,
        1
    );
    output << ",\n";

    output << "  \"robustness\": ";
    append_robustness(
        output,
        result.robustness,
        1
    );
    output << '\n';

    output << "}\n";

    return output.str();
}

void write_research_experiment_json(
    const ResearchExperimentResult& result,
    const std::string& file_path
) {
    if (file_path.empty()) {
        throw std::invalid_argument(
            "Research report path must not be empty"
        );
    }

    const std::string json =
        research_experiment_to_json(result);

    std::ofstream output(
        file_path,
        std::ios::binary |
        std::ios::trunc
    );

    if (!output) {
        throw std::runtime_error(
            "Unable to open research report file"
        );
    }

    output << json;

    if (!output) {
        throw std::runtime_error(
            "Failed while writing research report"
        );
    }
}

} // namespace quant::research