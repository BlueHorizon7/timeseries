#pragma once

#include "quant/research/parameter_sensitivity.hpp"
#include "quant/research/walk_forward.hpp"

#include <cstddef>
#include <vector>

namespace quant::research {

struct WalkForwardParameterStabilityPoint {
    SensitivityParameter parameter{};
    double parameter_value{};

    std::vector<double> fold_total_returns;
    std::vector<double> fold_sharpes;

    double mean_total_return{};
    double standard_deviation_total_return{};

    double mean_sharpe{};
    double standard_deviation_sharpe{};

    double mean_total_return_rank{};
    double standard_deviation_total_return_rank{};

    double mean_sharpe_rank{};
    double standard_deviation_sharpe_rank{};

    std::size_t number_of_observed_folds{};
};

struct WalkForwardParameterStabilityResult {
    SensitivityParameter parameter{};
    std::vector<WalkForwardParameterStabilityPoint> points;
    std::size_t number_of_folds{};
};

[[nodiscard]]
WalkForwardParameterStabilityResult
analyze_walk_forward_parameter_stability(
    const quant::data::AlignedSeries& data,
    const WalkForwardParameters& baseline_parameters,
    double periods_per_year,
    SensitivityParameter parameter,
    const std::vector<double>& values
);

} // namespace quant::research