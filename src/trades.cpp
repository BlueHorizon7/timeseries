#include "quant/risk/trades.hpp"

namespace quant::risk {

std::vector<Trade>
extract_trades(
    const quant::backtest::BacktestResult& result
) {
    std::vector<Trade> trades;

    bool in_trade = false;

    int direction = 0;

    std::int64_t entry_timestamp = 0;

    double entry_equity = 0.0;

    double accumulated_pnl = 0.0;

    for (const auto& bar : result.bars) {
        const int current_direction =
            bar.signal;

        if (!in_trade &&
            current_direction != 0) {

            in_trade = true;
            direction = current_direction;
            entry_timestamp = bar.timestamp;
            entry_equity = bar.equity;
            accumulated_pnl = bar.net_pnl;

            continue;
        }

        if (in_trade) {
            accumulated_pnl += bar.net_pnl;

            if (current_direction == 0 ||
                current_direction != direction) {

                trades.push_back(
                    Trade{
                        entry_timestamp,
                        bar.timestamp,
                        direction,
                        accumulated_pnl,
                        accumulated_pnl / entry_equity
                    }
                );

                in_trade = false;
                direction = 0;
                accumulated_pnl = 0.0;
            }
        }
    }

    return trades;
}

} // namespace quant::risk