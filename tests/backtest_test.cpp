#include "quant/backtest/backtest.hpp"

#include <cassert>
#include <cmath>

int main() {
    using quant::backtest::BacktestParameters;
    using quant::portfolio::TransactionCostParameters;

    const std::vector<std::int64_t> timestamps{
        1000,
        2000,
        3000
    };

    const std::vector<double> x_prices{
        100.0,
        110.0,
        110.0
    };

    const std::vector<double> y_prices{
        100.0,
        100.0,
        105.0
    };

    /*
        At t=0:

            signal = +1
            beta   = 1

        Therefore:

            X = -50000
            Y = +50000

        X rises 10%:

            X P&L = -5000

        Y unchanged:

            Y P&L = 0

        Gross P&L = -5000
    */

    const std::vector<int> signals{
        1,
        0,
        0
    };

    const std::vector<double> hedge_ratios{
        1.0,
        1.0,
        1.0
    };

    const BacktestParameters parameters{
        100000.0,
        100000.0,
        TransactionCostParameters{
            0.001,
            0.001
        }
    };

    const auto result =
        quant::backtest::run_backtest(
            timestamps,
            x_prices,
            y_prices,
            signals,
            hedge_ratios,
            parameters
        );

    assert(result.bars.size() == 2);

    /*
        Opening position:

            X = -50000
            Y = +50000

        Opening transaction cost:

            50000(0.001)
          + 50000(0.001)
          = 100
    */

    assert(
        std::abs(
            result.bars[0].transaction_cost - 100.0
        ) < 1e-12
    );

    assert(
        std::abs(
            result.bars[0].gross_pnl + 5000.0
        ) < 1e-12
    );

    assert(
        std::abs(
            result.bars[0].net_pnl + 5100.0
        ) < 1e-12
    );

    assert(
        std::abs(
            result.bars[0].equity - 94900.0
        ) < 1e-12
    );

    /*
        At t=1 signal = 0.

        Position changes from:

            (-50000, +50000)

        to:

            (0, 0)

        So the position is closed before
        the second interval.

        Closing cost = 100.

        No position is held during [t=1,t=2],
        therefore gross P&L = 0.
    */

    assert(
        std::abs(
            result.bars[1].gross_pnl
        ) < 1e-12
    );

    assert(
        std::abs(
            result.bars[1].transaction_cost - 100.0
        ) < 1e-12
    );

    assert(
        std::abs(
            result.bars[1].net_pnl + 100.0
        ) < 1e-12
    );

    assert(
        std::abs(
            result.bars[1].equity - 94800.0
        ) < 1e-12
    );

    return 0;
}