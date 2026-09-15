#include "quant/research/robustness_report.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::research {

namespace {

void validate_criteria(
    const RobustnessCriteria& criteria
) {
    if (!std::isfinite(
            criteria.minimum_break_even_cost_multiplier)) {

        throw std::invalid_argument(
            "Minimum break-even cost multiplier must be finite"
        );
    }

    if (criteria.minimum_profitable_subperiod_fraction < 0.0 ||
        criteria.minimum_profitable_subperiod_fraction > 1.0) {

        throw std::invalid_argument(
            "Subperiod profitability fraction must be in [0, 1]"
        );
    }

    if (criteria.minimum_profitable_parameter_fraction < 0.0 ||
        criteria.minimum_profitable_parameter_fraction > 1.0) {

        throw std::invalid_argument(
            "Parameter profitability fraction must be in [0, 1]"
        );
    }

    if (
        criteria.minimum_profitable_walk_forward_parameter_fraction < 0.0 ||
        criteria.minimum_profitable_walk_forward_parameter_fraction > 1.0
    ) {
        throw std::invalid_argument(
            "Walk-forward parameter profitability fraction "
            "must be in [0, 1]"
        );
    }

    if (!std::isfinite(
            criteria.minimum_bootstrap_sharpe_lower_bound)) {

        throw std::invalid_argument(
            "Minimum bootstrap Sharpe bound must be finite"
        );
    }

    if (criteria.maximum_placebo_sharpe_p_value <= 0.0 ||
        criteria.maximum_placebo_sharpe_p_value > 1.0) {

        throw std::invalid_argument(
            "Maximum placebo p-value must be in (0, 1]"
        );
    }
}

bool assess_cost_robustness(
    const CostSensitivityResult& result,
    const RobustnessCriteria& criteria
) {
    if (result.points.empty()) {
        return false;
    }

    if (!result.break_even_found) {
        return false;
    }

    if (!std::isfinite(
            result.break_even_cost_multiplier)) {

        return false;
    }

    return
        result.break_even_cost_multiplier >=
        criteria.minimum_break_even_cost_multiplier;
}

bool assess_parameter_robustness(
    const ParameterSensitivityResult& result,
    const RobustnessCriteria& criteria
) {
    if (result.points.empty()) {
        return false;
    }

    std::size_t profitable = 0;

    for (const auto& point : result.points) {
        if (!std::isfinite(
                point.performance.total_return)) {

            return false;
        }

        if (point.performance.total_return >= 0.0) {
            ++profitable;
        }
    }

    const double fraction =
        static_cast<double>(profitable) /
        static_cast<double>(result.points.size());

    return
        fraction >=
        criteria.minimum_profitable_parameter_fraction;
}

bool assess_subperiod_robustness(
    const SubperiodStabilityResult& result,
    const RobustnessCriteria& criteria
) {
    if (result.periods.empty()) {
        return false;
    }

    if (result.profitable_periods +
            result.losing_periods !=
        result.periods.size()) {

        return false;
    }

    const double fraction =
        static_cast<double>(
            result.profitable_periods
        ) /
        static_cast<double>(
            result.periods.size()
        );

    return
        fraction >=
        criteria.minimum_profitable_subperiod_fraction;
}

bool assess_walk_forward_parameter_robustness(
    const WalkForwardParameterStabilityResult& result,
    const RobustnessCriteria& criteria
) {
    if (result.points.empty()) {
        return false;
    }

    std::size_t profitable = 0;

    for (const auto& point : result.points) {
        if (point.number_of_observed_folds == 0) {
            return false;
        }

        if (!std::isfinite(
                point.mean_total_return)) {

            return false;
        }

        if (point.mean_total_return >= 0.0) {
            ++profitable;
        }
    }

    const double fraction =
        static_cast<double>(profitable) /
        static_cast<double>(result.points.size());

    return
        fraction >=
        criteria.minimum_profitable_walk_forward_parameter_fraction;
}

bool assess_bootstrap(
    const BootstrapResult& result,
    const RobustnessCriteria& criteria
) {
    if (result.observations == 0 ||
        result.number_of_resamples == 0) {

        return false;
    }

    if (result.sharpe_ratio_distribution.empty()) {
        return false;
    }

    if (!std::isfinite(
            result.sharpe_ratio_interval.lower)) {

        return false;
    }

    return
        result.sharpe_ratio_interval.lower >=
        criteria.minimum_bootstrap_sharpe_lower_bound;
}

bool assess_placebo(
    const PlaceboResult& result,
    const RobustnessCriteria& criteria
) {
    if (result.number_of_successful_placebos <
        criteria.minimum_successful_placebos) {

        return false;
    }

    if (!std::isfinite(
            result.empirical_p_value_sharpe_ratio)) {

        return false;
    }

    return
        result.empirical_p_value_sharpe_ratio <=
        criteria.maximum_placebo_sharpe_p_value;
}

} // namespace

RobustnessAssessment assess_robustness(
    const CostSensitivityResult& cost_sensitivity,
    const ParameterSensitivityResult& parameter_sensitivity,
    const SubperiodStabilityResult& subperiod_stability,
    const WalkForwardParameterStabilityResult&
        walk_forward_parameter_stability,
    const BootstrapResult& bootstrap,
    const PlaceboResult& placebo,
    const RobustnessCriteria& criteria
) {
    validate_criteria(criteria);

    RobustnessAssessment assessment{};

    assessment.cost_robust =
        assess_cost_robustness(
            cost_sensitivity,
            criteria
        );

    assessment.parameter_robust =
        assess_parameter_robustness(
            parameter_sensitivity,
            criteria
        );

    assessment.subperiod_robust =
        assess_subperiod_robustness(
            subperiod_stability,
            criteria
        );

    assessment.walk_forward_parameter_robust =
        assess_walk_forward_parameter_robustness(
            walk_forward_parameter_stability,
            criteria
        );

    assessment.bootstrap_supportive =
        assess_bootstrap(
            bootstrap,
            criteria
        );

    assessment.placebo_supportive =
        assess_placebo(
            placebo,
            criteria
        );

    assessment.checks_total = 6;

    assessment.checks_passed =
        static_cast<std::size_t>(
            assessment.cost_robust
        ) +
        static_cast<std::size_t>(
            assessment.parameter_robust
        ) +
        static_cast<std::size_t>(
            assessment.subperiod_robust
        ) +
        static_cast<std::size_t>(
            assessment.walk_forward_parameter_robust
        ) +
        static_cast<std::size_t>(
            assessment.bootstrap_supportive
        ) +
        static_cast<std::size_t>(
            assessment.placebo_supportive
        );

    if (assessment.checks_passed == assessment.checks_total) {
        assessment.status =
            RobustnessStatus::Robust;
    } else if (assessment.checks_passed == 0) {
        assessment.status =
            RobustnessStatus::Failed;
    } else {
        /*
            Partial survival of the robustness battery is not
            equivalent to statistical confirmation.
        */
        assessment.status =
            RobustnessStatus::Fragile;
    }

    if (!assessment.cost_robust) {
        assessment.warnings.push_back(
            "Cost sensitivity did not satisfy the configured criterion."
        );
    }

    if (!assessment.parameter_robust) {
        assessment.warnings.push_back(
            "Parameter sensitivity did not satisfy the configured criterion."
        );
    }

    if (!assessment.subperiod_robust) {
        assessment.warnings.push_back(
            "Subperiod stability did not satisfy the configured criterion."
        );
    }

    if (!assessment.walk_forward_parameter_robust) {
        assessment.warnings.push_back(
            "Walk-forward parameter stability did not satisfy "
            "the configured criterion."
        );
    }

    if (!assessment.bootstrap_supportive) {
        assessment.warnings.push_back(
            "Bootstrap uncertainty did not satisfy the configured criterion."
        );
    }

    if (!assessment.placebo_supportive) {
        assessment.warnings.push_back(
            "Placebo analysis did not satisfy the configured criterion."
        );
    }

    if (criteria.require_all_diagnostics &&
        assessment.checks_total != 6) {

        assessment.status =
            RobustnessStatus::Inconclusive;
    }

    return assessment;
}

} // namespace quant::research