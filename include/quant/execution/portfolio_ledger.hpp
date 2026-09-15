#pragma once

#include "quant/execution/fill.hpp"
#include "quant/execution/position.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace quant::execution {

struct PortfolioLedger {
    double cash{};

    double realized_pnl{};
    double unrealized_pnl{};
    double total_commission{};

    std::int64_t last_update_timestamp{};

    std::unordered_map<std::string, Position>
        positions;
};

void apply_fill(
    PortfolioLedger& ledger,
    const Fill& fill
);

void mark_to_market(
    PortfolioLedger& ledger,
    const std::unordered_map<std::string, double>&
        market_prices,
    std::int64_t timestamp
);

[[nodiscard]]
const Position& position(
    const PortfolioLedger& ledger,
    const std::string& symbol
);

[[nodiscard]]
double equity(
    const PortfolioLedger& ledger,
    const std::unordered_map<std::string, double>&
        market_prices
);

[[nodiscard]]
std::vector<Position> positions(
    const PortfolioLedger& ledger
);

} // namespace quant::execution