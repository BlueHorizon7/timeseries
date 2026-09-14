#include "quant/statistics/multiple_testing.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace quant::statistics {

namespace {

struct RankedPValue {
    double value{};
    std::size_t original_index{};
};

} // namespace

MultipleTestingResult benjamini_hochberg(
    const std::vector<double>& p_values,
    double false_discovery_rate
) {
    if (!std::isfinite(false_discovery_rate) ||
        false_discovery_rate <= 0.0 ||
        false_discovery_rate >= 1.0) {

        throw std::invalid_argument(
            "False discovery rate must be strictly "
            "between zero and one"
        );
    }

    if (p_values.empty()) {
        throw std::invalid_argument(
            "Benjamini-Hochberg requires at least "
            "one p-value"
        );
    }

    std::vector<RankedPValue> ranked;
    ranked.reserve(p_values.size());

    for (std::size_t i = 0;
         i < p_values.size();
         ++i) {

        const double p = p_values[i];

        if (!std::isfinite(p) ||
            p < 0.0 ||
            p > 1.0) {

            throw std::invalid_argument(
                "P-values must be finite and lie "
                "in the interval [0, 1]"
            );
        }

        ranked.push_back(
            RankedPValue{
                p,
                i
            }
        );
    }

    /*
        Stable deterministic ordering:

          1. ascending p-value
          2. ascending original index for ties
    */
    std::sort(
        ranked.begin(),
        ranked.end(),
        [](const RankedPValue& lhs,
           const RankedPValue& rhs) {

            if (lhs.value != rhs.value) {
                return lhs.value <
                       rhs.value;
            }

            return lhs.original_index <
                   rhs.original_index;
        }
    );

    const std::size_t m =
        ranked.size();

    std::vector<double> adjusted(
        m,
        1.0
    );

    /*
        Raw BH adjusted value:

            p_(k) * m / k

        where k is the 1-based rank.

        The monotonicity correction is then applied
        backwards:

            q_(k) =
                min(q_(k), q_(k+1))
    */
    double running_min =
        std::numeric_limits<double>::infinity();

    for (std::size_t r = m;
         r-- > 0;) {

        const std::size_t rank = r + 1;

        const double raw_adjusted =
            ranked[r].value *
            static_cast<double>(m) /
            static_cast<double>(rank);

        const double corrected =
            std::min(
                raw_adjusted,
                running_min
            );

        running_min = corrected;

        adjusted[
            ranked[r].original_index
        ] = std::clamp(
            corrected,
            0.0,
            1.0
        );
    }

    /*
        Determine the largest rank satisfying:

            p_(k) <= q * k / m

        Every hypothesis up to that rank is rejected.
    */
    std::size_t largest_rejected_rank = 0;

    for (std::size_t r = 0;
         r < m;
         ++r) {

        const std::size_t rank = r + 1;

        const double threshold =
            false_discovery_rate *
            static_cast<double>(rank) /
            static_cast<double>(m);

        if (ranked[r].value <= threshold) {
            largest_rejected_rank = rank;
        }
    }

    std::vector<bool> rejected(
        m,
        false
    );

    for (std::size_t r = 0;
         r < largest_rejected_rank;
         ++r) {

        rejected[
            ranked[r].original_index
        ] = true;
    }

    return MultipleTestingResult{
        std::move(adjusted),
        std::move(rejected),
        largest_rejected_rank
    };
}

} // namespace quant::statistics