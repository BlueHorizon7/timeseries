#include "quant/execution/portfolio_ledger.hpp"

#include <algorithm>
#include <stdexcept>

namespace quant::execution {

void apply_fill(
    PortfolioLedger& ledger,
    const Fill& fill
) {
    validate_fill(fill);

    Position& position =
        ledger.positions[fill.symbol];

    if (position.symbol.empty()) {
        position.symbol =
            fill.symbol;
    }

    const double signed_cash_flow =
        fill.side == OrderSide::Buy
            ? -fill.quantity * fill.price
            : fill.quantity * fill.price;

    ledger.cash +=
        signed_cash_flow -
        fill.commission;

    apply_position_fill(
        position,
        fill.side == OrderSide::Buy,
        fill.quantity,
        fill.price,
        fill.commission,
        fill.timestamp
    );

    ledger.realized_pnl +=
        position.realized_pnl -
        ledger.realized_pnl;

    ledger.total_commission +=
        fill.commission;

    ledger.last_update_timestamp =
        fill.timestamp;
}

void mark_to_market(
    PortfolioLedger& ledger,
    const std::unordered_map<std::string, double>&
        market_prices,
    std::int64_t timestamp
) {
    if (timestamp < ledger.last_update_timestamp) {
        throw std::invalid_argument(
            "Ledger timestamp cannot move backwards"
        );
    }

    double total_unrealized = 0.0;

    for (auto& [symbol, position] :
         ledger.positions) {

        const auto iterator =
            market_prices.find(symbol);

        if (iterator == market_prices.end()) {
            throw std::invalid_argument(
                "Missing market price for position: " +
                symbol
            );
        }

        mark_position(
            position,
            iterator->second,
            timestamp
        );

        total_unrealized +=
            position.unrealized_pnl;
    }

    ledger.unrealized_pnl =
        total_unrealized;

    ledger.last_update_timestamp =
        timestamp;
}

const Position& position(
    const PortfolioLedger& ledger,
    const std::string& symbol
) {
    const auto iterator =
        ledger.positions.find(symbol);

    if (iterator == ledger.positions.end()) {
        throw std::out_of_range(
            "Unknown position symbol: " +
            symbol
        );
    }

    return iterator->second;
}

double equity(
    const PortfolioLedger& ledger,
    const std::unordered_map<std::string, double>&
        market_prices
) {
    double result =
        ledger.cash;

    for (const auto& [symbol, position] :
         ledger.positions) {

        const auto iterator =
            market_prices.find(symbol);

        if (iterator == market_prices.end()) {
            throw std::invalid_argument(
                "Missing market price for position: " +
                symbol
            );
        }

        result +=
            position.quantity *
            iterator->second;
    }

    return result;
}

std::vector<Position> positions(
    const PortfolioLedger& ledger
) {
    std::vector<Position> result;

    result.reserve(
        ledger.positions.size()
    );

    for (const auto& [symbol, position] :
         ledger.positions) {

        (void)symbol;
        result.push_back(position);
    }

    std::sort(
        result.begin(),
        result.end(),
        [](const Position& lhs,
           const Position& rhs) {
            return lhs.symbol < rhs.symbol;
        }
    );

    return result;
}

} // namespace quant::execution