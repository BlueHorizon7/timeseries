#pragma once

#include "quant/math/cointegration.hpp"
#include "quant/research/pair_universe.hpp"

#include <cstddef>
#include <vector>

namespace quant::research {

struct PairSelectionParameters {
    PairUniverseParameters universe{};

    quant::math::CointegrationParameters
        cointegration{};

    double false_discovery_rate{0.05};
};

struct PairSelectionEntry {
    PairCandidate candidate{};

    quant::math::CointegrationResult
        cointegration{};

    double adjusted_p_value{};

    bool rejected{};
};

struct PairSelectionResult {
    std::vector<PairSelectionEntry>
        tested;

    std::vector<PairSelectionEntry>
        selected;
};

[[nodiscard]]
PairSelectionResult select_pairs(
    const std::vector<AssetSeries>& universe,
    const PairSelectionParameters&
        parameters
);

} // namespace quant::research