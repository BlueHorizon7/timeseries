#include "quant/research/parameter_grid.hpp"

#include <stdexcept>

namespace quant::research {

std::vector<StrategyParameterSet>
make_parameter_grid(
    const std::vector<std::size_t>& hedge_windows,
    const std::vector<std::size_t>& zscore_windows,
    const std::vector<double>& entry_zscores,
    const std::vector<double>& exit_zscores
) {
    if (hedge_windows.empty() ||
        zscore_windows.empty() ||
        entry_zscores.empty() ||
        exit_zscores.empty()) {
        throw std::invalid_argument(
            "Parameter grids must not be empty"
        );
    }

    std::vector<StrategyParameterSet> result;

    for (const auto hedge_window : hedge_windows) {
        for (const auto zscore_window : zscore_windows) {
            for (const auto entry : entry_zscores) {
                for (const auto exit : exit_zscores) {

                    if (hedge_window < 2 ||
                        zscore_window < 2 ||
                        entry <= 0.0 ||
                        exit < 0.0 ||
                        exit >= entry) {
                        throw std::invalid_argument(
                            "Invalid strategy parameter"
                        );
                    }

                    result.push_back(
                        StrategyParameterSet{
                            hedge_window,
                            zscore_window,
                            entry,
                            exit
                        }
                    );
                }
            }
        }
    }

    return result;
}

} // namespace quant::research