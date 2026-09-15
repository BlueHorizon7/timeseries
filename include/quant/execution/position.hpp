#pragma once

#include <cstdint>
#include <string>

namespace quant::execution {

struct Position {
    std::string symbol;

    double quantity{};
    double average_entry_price{};

    double realized_pnl{};
    double unrealized_pnl{};

    double total_commission{};

    std::int64_t last_update_timestamp{};
};

void apply_position_fill(
    Position& position,
    bool is_buy,
    double fill_quantity,
    double fill_price,
    double commission,
    std::int64_t timestamp
);

void mark_position(
    Position& position,
    double market_price,
    std::int64_t timestamp
);

} // namespace quant::execution