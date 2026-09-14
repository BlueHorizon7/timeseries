#pragma once

#include "quant/data/aligned_series.hpp"
#include "quant/research/walk_forward.hpp"

#include <cstddef>
#include <vector>

namespace quant::research {

enum class SensitivityParameter {
    HedgeRatioWindow,
    ZScoreWindow,
    EntryZScore,
    ExitZScore
};

struct ParameterSensitivityPoint {
    SensitivityParameter parameter{};
    double parameter_value{};

    quant::risk::PerformanceMetrics performance{};
};

struct ParameterSensitivityResult {
    SensitivityParameter parameter{};

    std::vector<
        ParameterSensitivityPoint
    > points;
};

[[nodiscard]]
ParameterSensitivityResult analyze_parameter_sensitivity(
    const quant::data::AlignedSeries& data,
    const WalkForwardParameters& baseline_parameters,
    double periods_per_year,
    SensitivityParameter parameter,
    const std::vector<double>& values
);

} // namespace quant::research