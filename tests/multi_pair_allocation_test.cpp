#include "quant/portfolio/multi_pair_allocation.hpp"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <vector>
#include <stdexcept>

namespace {

quant::research::PairSelectionResult
make_selection(
    std::size_t count
) {
    quant::research::PairSelectionResult result;

    result.selected.reserve(count);

    for (std::size_t i = 0;
         i < count;
         ++i) {

        result.selected.push_back(
            quant::research::PairSelectionEntry{}
        );
    }

    return result;
}

bool approximately_equal(
    double lhs,
    double rhs,
    double tolerance = 1e-12
) {
    return
        std::abs(lhs - rhs) <= tolerance;
}

} // namespace

int main() {
    using quant::portfolio::
        MultiPairAllocationParameters;

    using quant::portfolio::
        allocate_equal_gross_notional;

    /*
     * Four selected pairs.
     *
     * 1,000,000 capital
     * 80% gross allocation
     *
     * => 800,000 allocated
     * => 200,000 unallocated
     * => 200,000 per pair
     */
    const auto selection =
        make_selection(4);

    const MultiPairAllocationParameters
        parameters{
            1'000'000.0,
            0.80,
            0
        };

    const auto allocation =
        allocate_equal_gross_notional(
            selection,
            parameters
        );

    assert(
        approximately_equal(
            allocation.initial_capital,
            1'000'000.0
        )
    );

    assert(
        approximately_equal(
            allocation.allocated_capital,
            800'000.0
        )
    );

    assert(
        approximately_equal(
            allocation.unallocated_capital,
            200'000.0
        )
    );

    assert(
        approximately_equal(
            allocation.total_gross_notional,
            800'000.0
        )
    );

    assert(
        allocation.pairs.size() == 4
    );

    for (std::size_t i = 0;
         i < allocation.pairs.size();
         ++i) {

        const auto& pair =
            allocation.pairs[i];

        assert(
            pair.selected_pair_index == i
        );

        assert(
            approximately_equal(
                pair.weight,
                0.25
            )
        );

        assert(
            approximately_equal(
                pair.gross_notional,
                200'000.0
            )
        );
    }

    /*
     * Limiting the number of pairs.
     *
     * Only the first two selected pairs are
     * allocated.
     */
    const MultiPairAllocationParameters
        limited_parameters{
            1'000'000.0,
            1.0,
            2
        };

    const auto limited =
        allocate_equal_gross_notional(
            selection,
            limited_parameters
        );

    assert(
        limited.pairs.size() == 2
    );

    assert(
        approximately_equal(
            limited.total_gross_notional,
            1'000'000.0
        )
    );

    assert(
        approximately_equal(
            limited.pairs[0].weight,
            0.5
        )
    );

    assert(
        approximately_equal(
            limited.pairs[1].weight,
            0.5
        )
    );

    assert(
        approximately_equal(
            limited.pairs[0].gross_notional,
            500'000.0
        )
    );

    /*
     * Empty selection.
     */
    const auto empty_selection =
        make_selection(0);

    const auto empty_allocation =
        allocate_equal_gross_notional(
            empty_selection,
            parameters
        );

    assert(
        empty_allocation.pairs.empty()
    );

    assert(
        approximately_equal(
            empty_allocation.allocated_capital,
            0.0
        )
    );

    assert(
        approximately_equal(
            empty_allocation.unallocated_capital,
            1'000'000.0
        )
    );

    /*
     * Invalid capital.
     */
    bool threw = false;

    try {
        (void)
            allocate_equal_gross_notional(
                selection,
                MultiPairAllocationParameters{
                    0.0,
                    1.0,
                    0
                }
            );
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    /*
     * Invalid exposure fraction.
     */
    threw = false;

    try {
        (void)
            allocate_equal_gross_notional(
                selection,
                MultiPairAllocationParameters{
                    1'000'000.0,
                    1.1,
                    0
                }
            );
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    return 0;
}