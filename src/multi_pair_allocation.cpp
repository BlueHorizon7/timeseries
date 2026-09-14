#include "quant/portfolio/multi_pair_allocation.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::portfolio {

MultiPairAllocation
allocate_equal_gross_notional(
    const quant::research::PairSelectionResult&
        selection,
    const MultiPairAllocationParameters&
        parameters
) {
    if (!std::isfinite(
            parameters.initial_capital
        ) ||
        parameters.initial_capital <= 0.0) {

        throw std::invalid_argument(
            "Initial capital must be finite and positive"
        );
    }

    if (!std::isfinite(
            parameters.gross_exposure_fraction
        ) ||
        parameters.gross_exposure_fraction < 0.0 ||
        parameters.gross_exposure_fraction > 1.0) {

        throw std::invalid_argument(
            "Gross exposure fraction must be "
            "between zero and one"
        );
    }

    const std::size_t available_pairs =
        selection.selected.size();

    if (available_pairs == 0) {
        return MultiPairAllocation{
            parameters.initial_capital,
            0.0,
            parameters.initial_capital,
            0.0,
            {}
        };
    }

    std::size_t number_of_pairs =
        available_pairs;

    if (parameters.maximum_pairs != 0 &&
        number_of_pairs >
            parameters.maximum_pairs) {

        number_of_pairs =
            parameters.maximum_pairs;
    }

    if (number_of_pairs == 0) {
        throw std::invalid_argument(
            "Maximum pair count must not be zero"
        );
    }

    const double allocated_capital =
        parameters.initial_capital *
        parameters.gross_exposure_fraction;

    const double weight =
        1.0 /
        static_cast<double>(number_of_pairs);

    const double gross_notional_per_pair =
        allocated_capital * weight;

    MultiPairAllocation result;

    result.initial_capital =
        parameters.initial_capital;

    result.allocated_capital =
        allocated_capital;

    result.unallocated_capital =
        parameters.initial_capital -
        allocated_capital;

    result.total_gross_notional =
        gross_notional_per_pair *
        static_cast<double>(number_of_pairs);

    result.pairs.reserve(
        number_of_pairs
    );

    for (std::size_t i = 0;
         i < number_of_pairs;
         ++i) {

        result.pairs.push_back(
            PairAllocation{
                i,
                weight,
                gross_notional_per_pair
            }
        );
    }

    if (!std::isfinite(
            result.total_gross_notional
        )) {

        throw std::domain_error(
            "Portfolio gross notional is non-finite"
        );
    }

    return result;
}

} // namespace quant::portfolio