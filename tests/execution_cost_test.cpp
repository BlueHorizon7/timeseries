#include "quant/portfolio/execution_cost.hpp"

#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>

int main() {
    using quant::portfolio::ExecutionCostParameters;
    using quant::portfolio::PairPosition;

    {
        const PairPosition previous{
            0.0,
            0.0
        };

        const PairPosition current{
            -50000.0,
            50000.0
        };

        const ExecutionCostParameters parameters{
            10.0,
            20.0,

            5.0,
            10.0,

            100000.0,
            1.0
        };

        const auto result =
            quant::portfolio::calculate_execution_cost(
                previous,
                current,
                parameters
            );

        assert(
            std::abs(
                result.x_trade_notional - 50000.0
            ) < 1e-12
        );

        assert(
            std::abs(
                result.y_trade_notional - 50000.0
            ) < 1e-12
        );

        // X slippage: 50000 * 10 bps
        assert(
            std::abs(
                result.x_slippage_cost - 50.0
            ) < 1e-12
        );

        // Y slippage: 50000 * 20 bps
        assert(
            std::abs(
                result.y_slippage_cost - 100.0
            ) < 1e-12
        );

        // X impact:
        // 50000 * 5bps * (0.5)^1 = 12.5
        assert(
            std::abs(
                result.x_market_impact_cost - 12.5
            ) < 1e-12
        );

        // Y impact:
        // 50000 * 10bps * (0.5)^1 = 25
        assert(
            std::abs(
                result.y_market_impact_cost - 25.0
            ) < 1e-12
        );

        assert(
            std::abs(
                result.total_slippage_cost - 150.0
            ) < 1e-12
        );

        assert(
            std::abs(
                result.total_market_impact_cost - 37.5
            ) < 1e-12
        );

        assert(
            std::abs(
                result.total_execution_cost - 187.5
            ) < 1e-12
        );
    }

    {
        const PairPosition previous{
            1000.0,
            -2000.0
        };

        const PairPosition current{
            1000.0,
            -2000.0
        };

        const ExecutionCostParameters parameters{
            10.0,
            10.0,
            10.0,
            10.0,
            10000.0,
            1.0
        };

        const auto result =
            quant::portfolio::calculate_execution_cost(
                previous,
                current,
                parameters
            );

        assert(result.total_execution_cost == 0.0);
    }

    {
        const PairPosition previous{};
        const PairPosition current{
            1000.0,
            1000.0
        };

        const ExecutionCostParameters parameters{
            0.0,
            0.0,
            0.0,
            0.0,
            10000.0,
            0.5
        };

        const auto result =
            quant::portfolio::calculate_execution_cost(
                previous,
                current,
                parameters
            );

        assert(result.total_execution_cost == 0.0);
    }

    {
    const PairPosition previous{};
    const PairPosition current{
        1000.0,
        1000.0
    };

    ExecutionCostParameters parameters{};

    parameters.impact_reference_notional = 0.0;

    bool threw = false;

    try {
        (void)quant::portfolio::calculate_execution_cost(
            previous,
            current,
            parameters
        );
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);
}

    {
        const PairPosition previous{};
        const PairPosition current{};

        ExecutionCostParameters parameters{
            std::numeric_limits<double>::quiet_NaN(),
            0.0,
            0.0,
            0.0,
            10000.0,
            1.0
        };

        bool threw = false;

        try {
            (void)quant::portfolio::calculate_execution_cost(
                previous,
                current,
                parameters
            );
        } catch (const std::invalid_argument&) {
            threw = true;
        }

        assert(threw);
    }

    return 0;
}