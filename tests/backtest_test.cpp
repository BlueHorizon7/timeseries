#include "quant/backtest/backtest.hpp"

#include <cassert>
#include <cmath>
#include <vector>

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

    /*
        The position is already flat at the end,
        so no additional terminal liquidation cost
        is required.
    */

    assert(
        std::abs(
            result.liquidation_cost
        ) < 1e-12
    );

    /*
        =====================================================
        TERMINAL LIQUIDATION TEST
        =====================================================

        Here the position remains open through the
        final observed price.
    */
    {
        const std::vector<int> open_signals{
            1,
            1,
            1
        };

        const auto terminal_result =
            quant::backtest::run_backtest(
                timestamps,
                x_prices,
                y_prices,
                open_signals,
                hedge_ratios,
                parameters
            );

        assert(terminal_result.bars.size() == 2);

        /*
            Opening position:

                X = -50000
                Y = +50000

            Opening cost:

                50000 * 0.001
              + 50000 * 0.001
              = 100
        */

        assert(
            std::abs(
                terminal_result.bars[0].transaction_cost
                - 100.0
            ) < 1e-12
        );

        /*
            First interval:

                X:
                    -50000 * (110 - 100) / 100
                    = -5000

                Y:
                    50000 * (100 - 100) / 100
                    = 0

                Gross P&L = -5000
        */

        assert(
            std::abs(
                terminal_result.bars[0].gross_pnl
                + 5000.0
            ) < 1e-12
        );

        assert(
            std::abs(
                terminal_result.bars[0].net_pnl
                + 5100.0
            ) < 1e-12
        );

        assert(
            std::abs(
                terminal_result.bars[0].equity
                - 94900.0
            ) < 1e-12
        );

        /*
            Second interval:

                X does not move:

                    P&L = 0

                Y rises from 100 to 105:

                    50000 * 5 / 100
                    = +2500

                Gross P&L = +2500

            No position change occurs, so there is
            no trading transaction cost.
        */

        assert(
            std::abs(
                terminal_result.bars[1].gross_pnl
                - 2500.0
            ) < 1e-12
        );

        assert(
            std::abs(
                terminal_result.bars[1].transaction_cost
            ) < 1e-12
        );

        assert(
            std::abs(
                terminal_result.bars[1].net_pnl
                - 2500.0
            ) < 1e-12
        );

        /*
            Marked equity:

                100000
              -   100     opening cost
              -  5000     first interval P&L
              +  2500     second interval P&L
              = 97400
        */

        assert(
            std::abs(
                terminal_result.bars.back().equity
                - 97400.0
            ) < 1e-12
        );

        /*
            Terminal liquidation:

                50000 * 0.001
              + 50000 * 0.001
              = 100
        */

        assert(
            std::abs(
                terminal_result.liquidation_cost
                - 100.0
            ) < 1e-12
        );

        /*
            Fully liquidated final equity:

                97400 - 100
                = 97300
        */

        const double final_equity =
            terminal_result.bars.back().equity -
            terminal_result.liquidation_cost;

        assert(
            std::abs(
                final_equity - 97300.0
            ) < 1e-12
        );
    }

    return 0;
}