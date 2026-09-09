#include "quant/research/parameter_grid.hpp"

#include <cassert>

int main() {
    const auto grid =
        quant::research::make_parameter_grid(
            {20, 40},
            {10, 20},
            {1.5, 2.0},
            {0.5}
        );

    /*
     * 2 × 2 × 2 × 1 = 8 combinations.
     */
    assert(grid.size() == 8);

    assert(grid.front().hedge_ratio_window == 20);
    assert(grid.front().zscore_window == 10);
    assert(grid.front().entry_zscore == 1.5);
    assert(grid.front().exit_zscore == 0.5);

    return 0;
}