#include "quant/research/subperiod_stability.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>

int main() {
    std::cout << "A\n" << std::flush;

    using quant::backtest::BacktestBar;
    using quant::backtest::BacktestResult;

    std::cout << "B\n" << std::flush;

    BacktestResult backtest;

    /*
        Current BacktestBar layout:

            timestamp
            x_price
            y_price
            signal
            position
            gross_pnl
            transaction_cost
            execution_cost
            net_pnl
            equity
    */

    backtest.bars = {
        BacktestBar{
            1000,
            100.0,
            100.0,
            1,
            {},
            100.0,
            0.0,
            0.0,
            100.0,
            10100.0
        },

        BacktestBar{
            2000,
            101.0,
            101.0,
            1,
            {},
            200.0,
            0.0,
            0.0,
            200.0,
            10300.0
        },

        BacktestBar{
            3000,
            100.0,
            100.0,
            0,
            {},
            -100.0,
            0.0,
            0.0,
            -100.0,
            10200.0
        },

        BacktestBar{
            4000,
            103.0,
            103.0,
            0,
            {},
            300.0,
            0.0,
            0.0,
            300.0,
            10500.0
        }
    };

    std::cout << "C\n" << std::flush;

    const auto result =
        quant::research::analyze_subperiod_stability(
            backtest,
            252.0,
            2
        );

    std::cout << "D\n" << std::flush;

    assert(result.periods.size() == 2);

    /*
        First subperiod:
            initial equity = 10000
            P&L = 100 + 200 = 300
            final equity = 10300
            total return = 300 / 10000 = 0.03
    */

    assert(result.periods[0].begin_bar == 0);
    assert(result.periods[0].end_bar == 2);

    assert(
        std::abs(
            result.periods[0].performance.initial_equity -
            10000.0
        ) < 1e-12
    );

    assert(
        std::abs(
            result.periods[0].performance.final_equity -
            10300.0
        ) < 1e-12
    );

    assert(
        std::abs(
            result.periods[0].performance.total_pnl -
            300.0
        ) < 1e-12
    );

    assert(
        std::abs(
            result.periods[0].performance.total_return -
            0.03
        ) < 1e-12
    );

    /*
        Second subperiod:

            starts from equity = 10300
            P&L = -100 + 300 = 200
            final equity = 10500
    */

    assert(result.periods[1].begin_bar == 2);
    assert(result.periods[1].end_bar == 4);

    assert(
        std::abs(
            result.periods[1].performance.initial_equity -
            10300.0
        ) < 1e-12
    );

    assert(
        std::abs(
            result.periods[1].performance.final_equity -
            10500.0
        ) < 1e-12
    );

    assert(
        std::abs(
            result.periods[1].performance.total_pnl -
            200.0
        ) < 1e-12
    );

    assert(
        std::abs(
            result.periods[1].performance.total_return -
            (200.0 / 10300.0)
        ) < 1e-12
    );

    assert(result.profitable_periods == 2);
    assert(result.losing_periods == 0);

    assert(
        result.best_total_return >=
        result.worst_total_return
    );

    assert(
        result.best_sharpe >=
        result.worst_sharpe
    );

    /*
        Invalid number of subperiods.
    */

    bool threw_zero = false;

    try {
        (void)quant::research::analyze_subperiod_stability(
            backtest,
            252.0,
            0
        );
    } catch (const std::invalid_argument&) {
        threw_zero = true;
    }

    assert(threw_zero);

    bool threw_too_many = false;

    try {
        (void)quant::research::analyze_subperiod_stability(
            backtest,
            252.0,
            5
        );
    } catch (const std::invalid_argument&) {
        threw_too_many = true;
    }

    assert(threw_too_many);

    std::cout
        << "Subperiod stability tests passed!"
        << std::endl;

    return 0;
}