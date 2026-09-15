#pragma once

#include "quant/research/bootstrap.hpp"
#include "quant/research/cost_sensitivity.hpp"
#include "quant/research/parameter_sensitivity.hpp"
#include "quant/research/placebo.hpp"
#include "quant/research/subperiod_stability.hpp"
#include "quant/research/walk_forward_parameter_stability.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace quant::research {

enum class RobustnessStatus {
    Robust,
    Fragile,
    Inconclusive,
    Failed
};

struct RobustnessCriteria {
    /*
        Cost robustness:
        the tested cost multiplier at which the strategy breaks even
        must be at least this value.

        Example:
            1.0 means the strategy must survive at least baseline costs.
    */
    double minimum_break_even_cost_multiplier{1.0};

    /*
        Fraction of subperiods that must be profitable.
    */
    double minimum_profitable_subperiod_fraction{0.50};

    /*
        The best parameter point must not dominate all other points
        by an arbitrary amount. Instead, require at least this fraction
        of parameter points to have non-negative total return.

        This is deliberately configurable rather than treating a
        particular threshold as a universal statistical law.
    */
    double minimum_profitable_parameter_fraction{0.50};

    /*
        Walk-forward parameter stability:

        require at least this fraction of parameter points to have
        non-negative mean OOS total return.
    */
    double minimum_profitable_walk_forward_parameter_fraction{0.50};

    /*
        The bootstrap confidence interval lower bound for Sharpe
        must be at least this value.
    */
    double minimum_bootstrap_sharpe_lower_bound{0.0};

    /*
        Upper-tail empirical placebo p-value for Sharpe must be no
        greater than this threshold.
    */
    double maximum_placebo_sharpe_p_value{0.05};

    /*
        Minimum number of successful placebo runs.
    */
    std::size_t minimum_successful_placebos{1};

    /*
        Whether every required diagnostic must be evaluable.
    */
    bool require_all_diagnostics{true};
};

struct RobustnessAssessment {
    RobustnessStatus status{RobustnessStatus::Inconclusive};

    bool cost_robust{};
    bool parameter_robust{};
    bool subperiod_robust{};
    bool walk_forward_parameter_robust{};
    bool bootstrap_supportive{};
    bool placebo_supportive{};

    std::size_t checks_passed{};
    std::size_t checks_total{};

    std::vector<std::string> warnings;
};

[[nodiscard]]
RobustnessAssessment assess_robustness(
    const CostSensitivityResult& cost_sensitivity,
    const ParameterSensitivityResult& parameter_sensitivity,
    const SubperiodStabilityResult& subperiod_stability,
    const WalkForwardParameterStabilityResult&
        walk_forward_parameter_stability,
    const BootstrapResult& bootstrap,
    const PlaceboResult& placebo,
    const RobustnessCriteria& criteria
);

} // namespace quant::researchs