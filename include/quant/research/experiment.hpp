#pragma once

#include "quant/research/bootstrap.hpp"
#include "quant/research/cost_sensitivity.hpp"
#include "quant/research/parameter_sensitivity.hpp"
#include "quant/research/placebo.hpp"
#include "quant/research/robustness_report.hpp"
#include "quant/research/subperiod_stability.hpp"
#include "quant/research/walk_forward.hpp"
#include "quant/research/walk_forward_parameter_stability.hpp"
#include "quant/risk/performance.hpp"

#include <string>

namespace quant::research {

struct ResearchExperiment {
    std::string experiment_id;
    std::string description;

    WalkForwardParameters walk_forward_parameters;

    BootstrapParameters bootstrap_parameters;
    PlaceboParameters placebo_parameters;
    RobustnessCriteria robustness_criteria;

    double periods_per_year{};
};

struct ResearchExperimentResult {
    ResearchExperiment experiment;

    WalkForwardResult walk_forward;
    quant::risk::PerformanceMetrics performance;

    CostSensitivityResult cost_sensitivity;
    ParameterSensitivityResult parameter_sensitivity;
    SubperiodStabilityResult subperiod_stability;
    WalkForwardParameterStabilityResult
        walk_forward_parameter_stability;

    BootstrapResult bootstrap;
    PlaceboResult placebo;

    RobustnessAssessment robustness;
};

[[nodiscard]]
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
);

} // namespace quant::research