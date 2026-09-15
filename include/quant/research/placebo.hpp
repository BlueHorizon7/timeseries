#pragma once

#include "quant/research/walk_forward.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace quant::research {

struct PlaceboParameters {
    std::size_t number_of_placebos{500};
    std::uint64_t random_seed{123456789ULL};
    double periods_per_year{252.0};
};

struct PlaceboResult {
    double observed_total_return{};
    double observed_sharpe_ratio{};

    std::vector<double> placebo_total_returns;
    std::vector<double> placebo_sharpes;

    std::size_t number_of_placebos_requested{};
    std::size_t number_of_successful_placebos{};

    double empirical_p_value_total_return{};
    double empirical_p_value_sharpe_ratio{};
};

[[nodiscard]]
PlaceboResult analyze_circular_shift_placebo(
    const quant::data::AlignedSeries& data,
    const WalkForwardParameters& parameters,
    const PlaceboParameters& placebo_parameters
);

} // namespace quant::research