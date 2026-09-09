#pragma once

#include <cstddef>
#include <vector>

namespace quant::research {

struct StrategyParameterSet {
    std::size_t hedge_ratio_window{};
    std::size_t zscore_window{};
    double entry_zscore{};
    double exit_zscore{};
};

[[nodiscard]]
std::vector<StrategyParameterSet>
make_parameter_grid(
    const std::vector<std::size_t>& hedge_windows,
    const std::vector<std::size_t>& zscore_windows,
    const std::vector<double>& entry_zscores,
    const std::vector<double>& exit_zscores
);

} // namespace quant::research