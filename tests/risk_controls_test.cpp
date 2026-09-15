#include "quant/execution/risk_controls.hpp"

#include <cassert>
#include <iostream>

int main()
{
    using namespace quant::execution;

    const OperationalRiskLimits limits{
        .maximum_gross_notional = 10000.0,
        .maximum_absolute_position = 100.0,
        .maximum_orders_per_cycle = 5,
        .maximum_daily_loss = 1000.0
    };

    const auto accepted =
        validate_targets(
            {},
            {{"XYZ", 50.0}},
            {{"XYZ", 100.0}},
            limits,
            10000.0,
            9950.0
        );

    assert(accepted.approved);

    const auto rejected =
        validate_targets(
            {},
            {{"XYZ", 200.0}},
            {{"XYZ", 100.0}},
            limits,
            10000.0,
            9900.0
        );

    assert(!rejected.approved);

    std::cout << "risk_controls_test passed\n";
}