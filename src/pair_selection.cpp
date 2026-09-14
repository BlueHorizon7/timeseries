#include "quant/research/pair_selection.hpp"

#include "quant/math/cointegration.hpp"
#include "quant/statistics/multiple_testing.hpp"

#include <cmath>
#include <stdexcept>
#include <vector>

namespace quant::research {

PairSelectionResult select_pairs(
    const std::vector<AssetSeries>& universe,
    const PairSelectionParameters&
        parameters
) {
    if (!std::isfinite(
            parameters.false_discovery_rate
        ) ||
        parameters.false_discovery_rate <= 0.0 ||
        parameters.false_discovery_rate >= 1.0) {

        throw std::invalid_argument(
            "False discovery rate must be strictly "
            "between zero and one"
        );
    }

    const auto candidates =
        generate_pair_candidates(
            universe,
            parameters.universe
        );

    PairSelectionResult result;

    if (candidates.empty()) {
        return result;
    }

    result.tested.reserve(
        candidates.size()
    );

    std::vector<double> p_values;
    p_values.reserve(
        candidates.size()
    );

    /*
     * Every candidate that survives the correlation
     * pre-screen is subjected to the Engle-Granger
     * cointegration test.
     *
     * No multiple-testing decision is made yet.
     */
    for (const auto& candidate : candidates) {
        if (candidate.x_index >= universe.size() ||
            candidate.y_index >= universe.size()) {

            throw std::logic_error(
                "Pair candidate index is outside "
                "the asset universe"
            );
        }

        const auto& x =
            universe[candidate.x_index].prices;

        const auto& y =
            universe[candidate.y_index].prices;

        const auto cointegration =
            quant::math::engle_granger(
                x,
                y,
                parameters.cointegration
            );

        if (!std::isfinite(
                cointegration.p_value
            ) ||
            cointegration.p_value < 0.0 ||
            cointegration.p_value > 1.0) {

            throw std::domain_error(
                "Cointegration p-value is invalid"
            );
        }

        p_values.push_back(
            cointegration.p_value
        );

        result.tested.push_back(
            PairSelectionEntry{
                candidate,
                cointegration,
                1.0,
                false
            }
        );
    }

    /*
     * Multiple-testing correction is applied jointly
     * across all cointegration hypotheses that survived
     * the initial correlation screen.
     */
    const auto multiple_testing =
        quant::statistics::benjamini_hochberg(
            p_values,
            parameters.false_discovery_rate
        );

    result.selected.reserve(
        multiple_testing.number_rejected
    );

    for (std::size_t i = 0;
         i < result.tested.size();
         ++i) {

        result.tested[i].adjusted_p_value =
            multiple_testing.adjusted_p_values[i];

        result.tested[i].rejected =
            multiple_testing.rejected[i];

        if (result.tested[i].rejected) {
            result.selected.push_back(
                result.tested[i]
            );
        }
    }

    return result;
}

} // namespace quant::research