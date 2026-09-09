#include "quant/risk/trades.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>

int main() {
    using namespace quant::backtest;

    BacktestResult result;

    /*
     * Flat initially.
     */
    result.bars.push_back(
        BacktestBar{
            1000,
            100.0,
            100.0,
            0,
            {},
            0.0,
            0.0,
            0.0,
            10000.0
        }
    );

    /*
     * Enter a long-spread position.
     */
    result.bars.push_back(
        BacktestBar{
            2000,
            101.0,
            99.0,
            1,
            {500.0, -500.0},
            100.0,
            0.0,
            100.0,
            10100.0
        }
    );

    /*
     * Continue holding.
     */
    result.bars.push_back(
        BacktestBar{
            3000,
            102.0,
            98.0,
            1,
            {500.0, -500.0},
            100.0,
            0.0,
            100.0,
            10200.0
        }
    );

    /*
     * Exit.
     */
    result.bars.push_back(
        BacktestBar{
            4000,
            102.0,
            98.0,
            0,
            {},
            0.0,
            0.0,
            0.0,
            10200.0
        }
    );

    const auto trades =
        quant::risk::extract_trades(result);

    assert(trades.size() == 1);

    const auto& trade = trades[0];

    assert(trade.entry_timestamp == 2000);
    assert(trade.exit_timestamp == 4000);

    assert(trade.direction == 1);

    assert(std::abs(
        trade.pnl - 200.0
    ) < 1e-12);

    assert(std::abs(
        trade.return_pct -
        (200.0 / 10100.0)
    ) < 1e-12);

    return 0;
}