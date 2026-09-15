#include "quant/execution/portfolio_ledger.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace quant::execution {

void apply_fill(PortfolioLedger& ledger, const Fill& fill)
{
    validate_fill(fill);

    Position& position = ledger.positions[fill.symbol];

    if (position.symbol.empty()) {
        position.symbol = fill.symbol;
    }

    if (position.symbol != fill.symbol) {
        throw std::invalid_argument("position symbol mismatch");
    }

    if (!std::isfinite(ledger.cash) ||
        !std::isfinite(ledger.realized_pnl) ||
        !std::isfinite(ledger.unrealized_pnl) ||
        !std::isfinite(ledger.total_commission)) {
        throw std::invalid_argument("ledger state must be finite");
    }

    const double previous_realized = position.realized_pnl;

    const bool is_buy = fill.side == OrderSide::Buy;

    apply_position_fill(
        position,
        is_buy,
        fill.quantity,
        fill.price,
        fill.commission,
        fill.timestamp
    );

    const double notional = fill.quantity * fill.price;

    if (is_buy) {
        ledger.cash -= notional;
    } else {
        ledger.cash += notional;
    }

    ledger.cash -= fill.commission;

    ledger.realized_pnl +=
        position.realized_pnl - previous_realized;

    ledger.total_commission += fill.commission;
    ledger.last_update_timestamp = fill.timestamp;
}

void mark_to_market(
    PortfolioLedger& ledger,
    const std::unordered_map<std::string, double>& market_prices,
    std::int64_t timestamp)
{
    if (timestamp < 0) {
        throw std::invalid_argument("timestamp must be non-negative");
    }

    double total_unrealized = 0.0;

    for (auto& [symbol, position] : ledger.positions) {
        const auto it = market_prices.find(symbol);

        if (it == market_prices.end()) {
            throw std::invalid_argument(
                "missing market price for " + symbol
            );
        }

        if (!std::isfinite(it->second) || it->second <= 0.0) {
            throw std::invalid_argument(
                "market price must be finite and positive"
            );
        }

        mark_position(position, it->second, timestamp);

        total_unrealized += position.unrealized_pnl;
    }

    ledger.unrealized_pnl = total_unrealized;
    ledger.last_update_timestamp = timestamp;
}

const Position& position(
    const PortfolioLedger& ledger,
    const std::string& symbol)
{
    const auto it = ledger.positions.find(symbol);

    if (it == ledger.positions.end()) {
        throw std::out_of_range("position not found: " + symbol);
    }

    return it->second;
}

double equity(
    const PortfolioLedger& ledger,
    const std::unordered_map<std::string, double>& market_prices)
{
    double value = ledger.cash;

    for (const auto& [symbol, position_value] : ledger.positions) {
        const auto it = market_prices.find(symbol);

        if (it == market_prices.end()) {
            throw std::invalid_argument(
                "missing market price for " + symbol
            );
        }

        if (!std::isfinite(it->second) || it->second <= 0.0) {
            throw std::invalid_argument(
                "market price must be finite and positive"
            );
        }

        value += position_value.quantity * it->second;
    }

    return value;
}

std::vector<Position> positions(const PortfolioLedger& ledger)
{
    std::vector<Position> result;
    result.reserve(ledger.positions.size());

    for (const auto& [symbol, position_value] : ledger.positions) {
        static_cast<void>(symbol);
        result.push_back(position_value);
    }

    std::sort(
        result.begin(),
        result.end(),
        [](const Position& lhs, const Position& rhs) {
            return lhs.symbol < rhs.symbol;
        }
    );

    return result;
}

} // namespace quant::execution