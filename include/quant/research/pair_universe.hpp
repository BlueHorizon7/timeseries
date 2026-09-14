#pragma once

#include "quant/math/series.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace quant::research {

struct AssetSeries {
    std::string symbol;
    quant::math::Series prices;
};

struct PairCandidate {
    std::size_t x_index{};
    std::size_t y_index{};

    double correlation{};
    std::size_t observations{};
};

struct PairUniverseParameters {
    double minimum_absolute_correlation{0.70};

    /*
        Minimum number of aligned return observations required
        for a candidate to be considered.
    */
    std::size_t minimum_observations{30};

    /*
        0 means: retain every candidate passing the filter.
    */
    std::size_t maximum_candidates{};
};

[[nodiscard]]
std::vector<PairCandidate> generate_pair_candidates(
    const std::vector<AssetSeries>& universe,
    const PairUniverseParameters& parameters
);

} // namespace quant::research