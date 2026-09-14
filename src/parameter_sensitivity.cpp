#include "quant/research/parameter_sensitivity.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::research {

namespace {

std::size_t
as_window(double value) {
    if (!std::isfinite(value) ||
        value < 2.0 ||
        std::floor(value) != value) {
        throw std::invalid_argument(
            "Window sensitivity value must be an integer >= 2"
        );
    }

    return static_cast<std::size_t>(value);
}

void validate_value(
    SensitivityParameter parameter,
    double value
) {
    if (!std::isfinite(value)) {
        throw std::invalid_argument(
            "Sensitivity value must be finite"
        );
    }

    switch (parameter) {
        case SensitivityParameter::HedgeRatioWindow:
        case SensitivityParameter::ZScoreWindow:
            (void)as_window(value);
            break;

        case SensitivityParameter::EntryZScore:
            if (value <= 0.0) {
                throw std::invalid_argument(
                    "Entry z-score must be positive"
                );
            }
            break;

        case SensitivityParameter::ExitZScore:
            if (value < 0.0) {
                throw std::invalid_argument(
                    "Exit z-score must be non-negative"
                );
            }
            break;
    }
}

WalkForwardParameters
apply_parameter(
    const WalkForwardParameters& baseline,
    SensitivityParameter parameter,
    double value
) {
    WalkForwardParameters parameters =
        baseline;

    switch (parameter) {
        case SensitivityParameter::HedgeRatioWindow:
            parameters.strategy.hedge_ratio_window =
                as_window(value);
            break;

        case SensitivityParameter::ZScoreWindow:
            parameters.strategy.zscore_window =
                as_window(value);
            break;

        case SensitivityParameter::EntryZScore:
            parameters.strategy
                .signal_parameters
                .entry_zscore = value;
            break;

        case SensitivityParameter::ExitZScore:
            parameters.strategy
                .signal_parameters
                .exit_zscore = value;
            break;
    }

    return parameters;
}

} // namespace

ParameterSensitivityResult
analyze_parameter_sensitivity(
    const quant::data::AlignedSeries& data,
    const WalkForwardParameters& baseline_parameters,
    double periods_per_year,
    SensitivityParameter parameter,
    const std::vector<double>& values
) {
    if (data.size() < 3) {
        throw std::invalid_argument(
            "Parameter sensitivity requires at least three observations"
        );
    }

    if (!std::isfinite(periods_per_year) ||
        periods_per_year <= 0.0) {
        throw std::invalid_argument(
            "Periods per year must be finite and positive"
        );
    }

    if (values.empty()) {
        throw std::invalid_argument(
            "Parameter sensitivity requires at least one value"
        );
    }

    ParameterSensitivityResult result;
    result.parameter = parameter;
    result.points.reserve(values.size());

    for (const double value : values) {
        validate_value(
            parameter,
            value
        );

        auto parameters =
            apply_parameter(
                baseline_parameters,
                parameter,
                value
            );

        /*
         * Preserve every other research parameter.
         *
         * Only the selected parameter is changed.
         */
        const auto walk_forward =
            run_walk_forward(
                data,
                parameters,
                periods_per_year
            );

        if (!walk_forward.aggregate_performance.has_value()) {
            throw std::runtime_error(
                "Parameter sensitivity produced no aggregate OOS performance"
            );
        }

        result.points.push_back(
            ParameterSensitivityPoint{
                parameter,
                value,
                *walk_forward.aggregate_performance
            }
        );
    }

    return result;
}

} // namespace quant::research