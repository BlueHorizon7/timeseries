#include "quant/research/experiment.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace quant::research {

namespace {

void validate_experiment(
    const ResearchExperiment& experiment
) {
    if (experiment.experiment_id.empty()) {
        throw std::invalid_argument(
            "Research experiment requires a non-empty experiment id"
        );
    }

    if (experiment.periods_per_year <= 0.0 ||
        !std::isfinite(experiment.periods_per_year)) {

        throw std::invalid_argument(
            "Research experiment periods per year must be positive"
        );
    }

    if (experiment.walk_forward_parameters.formation_size == 0) {
        throw std::invalid_argument(
            "Research experiment formation size must be positive"
        );
    }

    if (experiment.walk_forward_parameters.test_size == 0) {
        throw std::invalid_argument(
            "Research experiment test size must be positive"
        );
    }
}

void validate_walk_forward_result(
    const ResearchExperiment& experiment,
    const WalkForwardResult& result
) {
    if (result.folds.empty()) {
        throw std::invalid_argument(
            "Research experiment requires at least one walk-forward fold"
        );
    }

    if (!result.aggregate_performance.has_value()) {
        throw std::invalid_argument(
            "Research experiment requires aggregate walk-forward performance"
        );
    }

    if (!result.aggregate_backtest.has_value()) {
        throw std::invalid_argument(
            "Research experiment requires aggregate walk-forward backtest"
        );
    }

    const auto& aggregate_performance =
        *result.aggregate_performance;

    if (!std::isfinite(
            aggregate_performance.total_return)) {

        throw std::invalid_argument(
            "Research experiment received non-finite aggregate total return"
        );
    }

    if (!std::isfinite(
            aggregate_performance.sharpe_ratio)) {

        throw std::invalid_argument(
            "Research experiment received non-finite aggregate Sharpe ratio"
        );
    }

    /*
        The supplied performance must correspond to the
        authoritative walk-forward aggregate.
    */
    const double tolerance = 1e-10;

    if (std::abs(
            aggregate_performance.final_equity -
            result.aggregate_performance->final_equity
        ) > tolerance) {

        throw std::invalid_argument(
            "Inconsistent walk-forward performance"
        );
    }

    (void)experiment;
}

void validate_bootstrap_result(
    const BootstrapResult& result
) {
    if (result.observations == 0) {
        throw std::invalid_argument(
            "Research experiment bootstrap has no observations"
        );
    }

    if (result.number_of_resamples == 0) {
        throw std::invalid_argument(
            "Research experiment bootstrap has no resamples"
        );
    }

    if (
        result.total_return_distribution.size() !=
        result.number_of_resamples
    ) {
        throw std::invalid_argument(
            "Research experiment bootstrap total-return distribution "
            "has an inconsistent size"
        );
    }

    if (
        result.sharpe_ratio_distribution.size() !=
        result.number_of_resamples
    ) {
        throw std::invalid_argument(
            "Research experiment bootstrap Sharpe distribution "
            "has an inconsistent size"
        );
    }
}

void validate_placebo_result(
    const PlaceboResult& result
) {
    if (result.number_of_placebos_requested == 0) {
        throw std::invalid_argument(
            "Research experiment placebo analysis has no requested runs"
        );
    }

    if (result.number_of_successful_placebos == 0) {
        throw std::invalid_argument(
            "Research experiment placebo analysis has no successful runs"
        );
    }

    if (
        result.placebo_total_returns.size() !=
        result.number_of_successful_placebos
    ) {
        throw std::invalid_argument(
            "Research experiment placebo total-return distribution "
            "has an inconsistent size"
        );
    }

    if (
        result.placebo_sharpes.size() !=
        result.number_of_successful_placebos
    ) {
        throw std::invalid_argument(
            "Research experiment placebo Sharpe distribution "
            "has an inconsistent size"
        );
    }

    if (!std::isfinite(
            result.empirical_p_value_sharpe_ratio)) {

        throw std::invalid_argument(
            "Research experiment placebo Sharpe p-value is not finite"
        );
    }
}

} // namespace

ResearchExperimentResult assemble_research_experiment(
    const ResearchExperiment& experiment,
    const WalkForwardResult& walk_forward,
    const quant::risk::PerformanceMetrics& performance,
    const CostSensitivityResult& cost_sensitivity,
    const ParameterSensitivityResult& parameter_sensitivity,
    const SubperiodStabilityResult& subperiod_stability,
    const WalkForwardParameterStabilityResult&
        walk_forward_parameter_stability,
    const BootstrapResult& bootstrap,
    const PlaceboResult& placebo
) {
    validate_experiment(experiment);

    validate_walk_forward_result(
        experiment,
        walk_forward
    );

    validate_bootstrap_result(
        bootstrap
    );

    validate_placebo_result(
        placebo
    );

    if (parameter_sensitivity.points.empty()) {
        throw std::invalid_argument(
            "Research experiment parameter sensitivity is empty"
        );
    }

    if (subperiod_stability.periods.empty()) {
        throw std::invalid_argument(
            "Research experiment subperiod stability is empty"
        );
    }

    if (walk_forward_parameter_stability.points.empty()) {
        throw std::invalid_argument(
            "Research experiment walk-forward parameter stability is empty"
        );
    }

    if (!std::isfinite(performance.total_return) ||
        !std::isfinite(performance.sharpe_ratio)) {

        throw std::invalid_argument(
            "Research experiment performance contains non-finite values"
        );
    }

    ResearchExperimentResult result{};

    result.experiment = experiment;

    result.walk_forward =
        walk_forward;

    result.performance =
        performance;

    result.cost_sensitivity =
        cost_sensitivity;

    result.parameter_sensitivity =
        parameter_sensitivity;

    result.subperiod_stability =
        subperiod_stability;

    result.walk_forward_parameter_stability =
        walk_forward_parameter_stability;

    result.bootstrap =
        bootstrap;

    result.placebo =
        placebo;

    result.robustness =
        assess_robustness(
            result.cost_sensitivity,
            result.parameter_sensitivity,
            result.subperiod_stability,
            result.walk_forward_parameter_stability,
            result.bootstrap,
            result.placebo,
            experiment.robustness_criteria
        );

    return result;
}

} // namespace quant::research