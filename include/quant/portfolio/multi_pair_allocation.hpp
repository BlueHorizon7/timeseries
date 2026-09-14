#pragma once

#include "quant/research/pair_selection.hpp"

#include <cstddef>
#include <vector>

namespace quant::portfolio {

struct MultiPairAllocationParameters {
    double initial_capital{};

    /*
     * Fraction of capital committed to the strategy.
     *
     * 1.0 means 100% of capital is allocated.
     */
    double gross_exposure_fraction{1.0};

    /*
     * 0 means: allocate across every selected pair.
     */
    std::size_t maximum_pairs{};
};

struct PairAllocation {
    std::size_t selected_pair_index{};

    double weight{};

    double gross_notional{};
};

struct MultiPairAllocation {
    double initial_capital{};

    double allocated_capital{};

    double unallocated_capital{};

    double total_gross_notional{};

    std::vector<PairAllocation> pairs;
};

[[nodiscard]]
MultiPairAllocation allocate_equal_gross_notional(
    const quant::research::PairSelectionResult&
        selection,
    const MultiPairAllocationParameters&
        parameters
);

} // namespace quant::portfolio