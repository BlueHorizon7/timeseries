#pragma once

#include "quant/portfolio/pair_position.hpp"

namespace quant::portfolio {

struct PnLResult {
    double x_pnl{};
    double y_pnl{};
    double total_pnl{};
};

[[nodiscard]]
PnLResult calculate_pnl(
    const PairPosition& position,
    double previous_x_price,
    double current_x_price,
    double previous_y_price,
    double current_y_price
);

} // namespace quant::portfolio