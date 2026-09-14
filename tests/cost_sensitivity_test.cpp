#include "quant/research/cost_sensitivity.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    using quant::backtest::BacktestParameters;
    using quant::portfolio::ExecutionCostParameters;
    using quant::portfolio::TransactionCostParameters;

    const std::vector<std::int64_t> timestamps{
        1000,
        2000,
        3000,
        4000
    };

    const std::vector<double> x_prices{
        100.0,
        110.0,
        105.0,
        100.0
    };

    const std::vector<double> y_prices{
        100.0,
        100.0,
        105.0,
        110.0
    };

    const std::vector<int> signals{
        1,
        0,
        -1,
        0
    };

    const std::vector<double> hedge_ratios{
        1.0,
        1.0,
        1.0,
        1.0
    };

    BacktestParameters parameters{};

    parameters.initial_capital = 100000.0;
    parameters.gross_notional = 100000.0;

    parameters.transaction_costs =
        TransactionCostParameters{
            0.001,
            0.001
        };

    parameters.execution_costs =
        ExecutionCostParameters{
            10.0,
            10.0,
            5.0,
            5.0,
            100000.0,
            1.0
        };

    const std::vector<double> multipliers{
        0.0,
        0.5,
        1.0,
        2.0,
        3.0
    };

    const auto result =
        quant::research::analyze_cost_sensitivity(
            timestamps,
            x_prices,
            y_prices,
            signals,
            hedge_ratios,
            parameters,
            252.0,
            multipliers
        );

    assert(result.points.size() == multipliers.size());

    for (std::size_t i = 0; i < result.points.size(); ++i) {
        assert(
            result.points[i].cost_multiplier ==
            multipliers[i]
        );

        assert(
            std::isfinite(
                result.points[i]
                    .performance.final_equity
            )
        );
    }

    assert(
        result.points.front()
            .total_transaction_cost == 0.0
    );

    assert(
        result.points.front()
            .total_execution_cost == 0.0
    );

    for (std::size_t i = 1; i < result.points.size(); ++i) {
        assert(
            result.points[i]
                .total_transaction_cost >=
            result.points[i - 1]
                .total_transaction_cost
        );

        assert(
            result.points[i]
                .total_execution_cost >=
            result.points[i - 1]
                .total_execution_cost
        );
    }

    std::cout
        << "Cost sensitivity tests passed!\n";

    return 0;
}