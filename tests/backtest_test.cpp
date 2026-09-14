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
        },

        quant::portfolio::ExecutionCostParameters{
            10.0,       // X slippage: 10 bps
            20.0,       // Y slippage: 20 bps
            5.0,        // X market impact: 5 bps
            10.0,       // Y market impact: 10 bps
            100000.0,   // reference notional
            1.0         // impact exponent
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

        Transaction cost:

            50000(0.001)
          + 50000(0.001)
          = 100
    */

    assert(
        std::abs(
            result.bars[0].transaction_cost - 100.0
        ) < 1e-12
    );

    /*
        Execution cost:

        X slippage:
            50000 * 10 / 10000 = 50

        Y slippage:
            50000 * 20 / 10000 = 100

        X market impact:
            50000 * 5 / 10000 * 0.5 = 12.5

        Y market impact:
            50000 * 10 / 10000 * 0.5 = 25

        Total:
            187.5
    */

    assert(
        std::abs(
            result.bars[0].execution_cost - 187.5
        ) < 1e-12
    );

    assert(
        std::abs(
            result.bars[0].gross_pnl + 5000.0
        ) < 1e-12
    );

    assert(
        std::abs(
            result.bars[0].net_pnl + 5287.5
        ) < 1e-12
    );

    assert(
        std::abs(
            result.bars[0].equity - 94712.5
        ) < 1e-12
    );

    /*
        At t=1 signal = 0.

        Position changes from:

            (-50000, +50000)

        to:

            (0, 0)

        Therefore:

            gross P&L = 0
            transaction cost = 100
            execution cost = 187.5
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
            result.bars[1].execution_cost - 187.5
        ) < 1e-12
    );

    assert(
        std::abs(
            result.bars[1].net_pnl + 287.5
        ) < 1e-12
    );

    assert(
        std::abs(
            result.bars[1].equity - 94425.0
        ) < 1e-12
    );

    /*
        The position is already flat at the end,
        so terminal liquidation cost is zero.
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
            Opening transaction cost = 100
        */

        assert(
            std::abs(
                terminal_result.bars[0].transaction_cost
                - 100.0
            ) < 1e-12
        );

        assert(
            std::abs(
                terminal_result.bars[0].execution_cost
                - 187.5
            ) < 1e-12
        );

        /*
            First interval:

                X P&L = -5000
                Y P&L = 0

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
                + 5287.5
            ) < 1e-12
        );

        assert(
            std::abs(
                terminal_result.bars[0].equity
                - 94712.5
            ) < 1e-12
        );

        /*
            Second interval:

                X does not move:
                    P&L = 0

                Y rises from 100 to 105:
                    50000 * 5 / 100
                    = +2500

            No position change occurs.
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
                terminal_result.bars[1].execution_cost
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
              -    100
              -    187.5
              -   5000
              +   2500
              = 97212.5
        */

        assert(
            std::abs(
                terminal_result.bars.back().equity
                - 97212.5
            ) < 1e-12
        );

        /*
            Terminal liquidation:

                Transaction cost = 100
                Execution cost   = 187.5

                Total = 287.5
        */

        assert(
            std::abs(
                terminal_result.liquidation_cost
                - 287.5
            ) < 1e-12
        );

        /*
            Fully liquidated final equity:

                97212.5 - 287.5
                = 96925.0
        */

        const double final_equity =
            terminal_result.bars.back().equity -
            terminal_result.liquidation_cost;

        assert(
            std::abs(
                final_equity - 96925.0
            ) < 1e-12
        );
    }

    return 0;
}