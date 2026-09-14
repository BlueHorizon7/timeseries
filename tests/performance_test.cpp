#include "quant/risk/performance.hpp"

#include <cassert>
#include <stdexcept>
#include <cmath>

int main() {
    using namespace quant::backtest;

    BacktestResult result;
    result.liquidation_cost = 0.0;

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
    0.0,
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

    assert(std::abs(
    metrics.annualized_volatility -
    1.0348685828430468
) < 1e-12);

assert(std::abs(
    metrics.sharpe_ratio -
    4.326569833978252
) < 1e-12);

    assert(metrics.maximum_drawdown > 0.0);

    assert(metrics.maximum_drawdown_pct > 0.0);

    assert(metrics.number_of_bars == 3);

    assert(metrics.number_of_position_changes == 2);

        /*
        Terminal liquidation must reduce final equity
        without being treated as another volatility
        period.
    */
    result.liquidation_cost = 10.0;

    const auto liquidated_metrics =
        quant::risk::calculate_performance(
            result,
            252.0
        );

    assert(std::abs(
        liquidated_metrics.initial_equity - 1000.0
    ) < 1e-12);

    assert(std::abs(
        liquidated_metrics.final_equity - 1040.0
    ) < 1e-12);

    assert(std::abs(
        liquidated_metrics.total_pnl - 40.0
    ) < 1e-12);

        result.liquidation_cost = -1.0;

    bool threw = false;

    try {
        (void)quant::risk::calculate_performance(
            result,
            252.0
        );
    } catch (const std::domain_error&) {
        threw = true;
    }

    assert(threw);

    result.liquidation_cost = 0.0;
    return 0;
}