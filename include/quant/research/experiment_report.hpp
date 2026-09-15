#pragma once

#include "quant/research/experiment.hpp"

#include <string>

namespace quant::research {

[[nodiscard]]
std::string robustness_status_to_string(
    RobustnessStatus status
);

[[nodiscard]]
std::string sensitivity_parameter_to_string(
    SensitivityParameter parameter
);

[[nodiscard]]
std::string research_experiment_to_json(
    const ResearchExperimentResult& result
);

void write_research_experiment_json(
    const ResearchExperimentResult& result,
    const std::string& file_path
);

} // namespace quant::research