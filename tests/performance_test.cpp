#include "quant/risk/performance.hpp"

#include <cassert>
#include <cmath>

int main() {
    using namespace quant::backtest;

    BacktestResult result;

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
            1000.0
        }
    );

    result.bars.push_back(
        BacktestBar{
            2000,
            101.0,
            101.0,
            1,
            {50.0, -50.0},
            100.0,
            10.0,
            90.0,
            1090.0
        }
    );

    result.bars.push_back(
        BacktestBar{
            3000,
            102.0,
            102.0,
            0,
            {},
            -40.0,
            0.0,
            -40.0,
            1050.0
        }
    );

    const auto metrics =
        quant::risk::calculate_performance(
            result,
            252.0
        );

    assert(std::abs(
        metrics.initial_equity - 1000.0
    ) < 1e-12);

    assert(std::abs(
        metrics.final_equity - 1050.0
    ) < 1e-12);

    assert(std::abs(
        metrics.total_pnl - 50.0
    ) < 1e-12);

    assert(std::abs(
        metrics.total_return - 0.05
    ) < 1e-12);

    assert(metrics.maximum_drawdown > 0.0);

    assert(metrics.maximum_drawdown_pct > 0.0);

    assert(metrics.number_of_bars == 3);

    assert(metrics.number_of_position_changes == 2);

    return 0;
}