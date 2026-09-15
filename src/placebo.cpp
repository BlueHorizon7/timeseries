#include "quant/research/placebo.hpp"

#include <cmath>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace quant::research {

namespace {

void validate_parameters(
    const quant::data::AlignedSeries& data,
    const WalkForwardParameters& parameters,
    const PlaceboParameters& placebo_parameters
) {
    if (data.size() < 2) {
        throw std::invalid_argument(
            "Placebo analysis requires at least two observations"
        );
    }

    if (placebo_parameters.number_of_placebos == 0) {
        throw std::invalid_argument(
            "Placebo analysis requires at least one placebo"
        );
    }

    if (placebo_parameters.periods_per_year <= 0.0 ||
        !std::isfinite(placebo_parameters.periods_per_year)) {

        throw std::invalid_argument(
            "Placebo periods per year must be positive"
        );
    }

    if (parameters.formation_size == 0 ||
        parameters.test_size == 0) {

        throw std::invalid_argument(
            "Walk-forward formation and test sizes must be positive"
        );
    }

    if (parameters.formation_size >= data.size()) {
        throw std::invalid_argument(
            "Formation period must leave observations for testing"
        );
    }
}

quant::data::AlignedSeries circular_shift_y(
    const quant::data::AlignedSeries& data,
    std::size_t shift
) {
    quant::data::AlignedSeries result;

    result.reserve(data.size());

    const std::size_t n = data.size();

    for (std::size_t i = 0; i < n; ++i) {
        const std::size_t source_index =
            (i + shift) % n;

        result.add(
            quant::data::AlignedObservation{
                data[i].timestamp,
                data[i].x,
                data[source_index].y
            }
        );
    }

    return result;
}

double upper_tail_empirical_p_value(
    const std::vector<double>& placebo_values,
    double observed_value
) {
    if (placebo_values.empty()) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    std::size_t extreme_count = 0;

    for (const double value : placebo_values) {
        if (value >= observed_value) {
            ++extreme_count;
        }
    }

    /*
        +1 correction prevents a zero p-value and is the standard
        finite-sample empirical-tail correction.
    */
    return
        (1.0 +
         static_cast<double>(extreme_count)) /
        (1.0 +
         static_cast<double>(placebo_values.size()));
}

} // namespace

PlaceboResult analyze_circular_shift_placebo(
    const quant::data::AlignedSeries& data,
    const WalkForwardParameters& parameters,
    const PlaceboParameters& placebo_parameters
) {
    validate_parameters(
        data,
        parameters,
        placebo_parameters
    );

    const auto observed =
        run_walk_forward(
            data,
            parameters,
            placebo_parameters.periods_per_year
        );

    if (!observed.aggregate_performance.has_value()) {
        throw std::runtime_error(
            "Observed walk-forward strategy has no aggregate performance"
        );
    }

    const double observed_total_return =
        observed.aggregate_performance->total_return;

    const double observed_sharpe =
        observed.aggregate_performance->sharpe_ratio;

    PlaceboResult result{};

    result.observed_total_return =
        observed_total_return;

    result.observed_sharpe_ratio =
        observed_sharpe;

    result.number_of_placebos_requested =
        placebo_parameters.number_of_placebos;

    result.placebo_total_returns.reserve(
        placebo_parameters.number_of_placebos
    );

    result.placebo_sharpes.reserve(
        placebo_parameters.number_of_placebos
    );

    std::mt19937_64 generator(
        placebo_parameters.random_seed
    );

    /*
        Avoid shift = 0 because that reproduces the observed data.
    */
    std::uniform_int_distribution<std::size_t>
        shift_distribution(
            1,
            data.size() - 1
        );

    for (std::size_t iteration = 0;
         iteration < placebo_parameters.number_of_placebos;
         ++iteration) {

        const std::size_t shift =
            shift_distribution(generator);

        const auto placebo_data =
            circular_shift_y(
                data,
                shift
            );

        const auto placebo_result =
            run_walk_forward(
                placebo_data,
                parameters,
                placebo_parameters.periods_per_year
            );

        if (!placebo_result.aggregate_performance.has_value()) {
            continue;
        }

        const double total_return =
            placebo_result.aggregate_performance
                ->total_return;

        const double sharpe =
            placebo_result.aggregate_performance
                ->sharpe_ratio;

        if (!std::isfinite(total_return) ||
            !std::isfinite(sharpe)) {
            continue;
        }

        result.placebo_total_returns.push_back(
            total_return
        );

        result.placebo_sharpes.push_back(
            sharpe
        );
    }

    result.number_of_successful_placebos =
        result.placebo_total_returns.size();

    if (result.number_of_successful_placebos == 0) {
        throw std::runtime_error(
            "No successful placebo walk-forward runs"
        );
    }

    result.empirical_p_value_total_return =
        upper_tail_empirical_p_value(
            result.placebo_total_returns,
            result.observed_total_return
        );

    result.empirical_p_value_sharpe_ratio =
        upper_tail_empirical_p_value(
            result.placebo_sharpes,
            result.observed_sharpe_ratio
        );

    return result;
}

} // namespace quant::research