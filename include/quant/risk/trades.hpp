#pragma once

#include "quant/backtest/backtest.hpp"

#include <cstddef>
#include <vector>

namespace quant::risk {

struct Trade {
    std::int64_t entry_timestamp{};
    std::int64_t exit_timestamp{};

    int direction{};

    double pnl{};
    double return_pct{};
};

[[nodiscard]]
std::vector<Trade>
extract_trades(
    const quant::backtest::BacktestResult& result
);

} // namespace quant::risk