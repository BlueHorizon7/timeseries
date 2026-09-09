#include "quant/portfolio/transaction_cost.hpp"

#include <cassert>
#include <cmath>
#include <stdexcept>

int main() {
    using quant::portfolio::PairPosition;
    using quant::portfolio::TransactionCostParameters;

    const PairPosition previous{
        -60000.0,
        40000.0
    };

    const PairPosition current{
        0.0,
        0.0
    };

    const TransactionCostParameters parameters{
        0.001,
        0.001
    };

    const double cost =
        quant::portfolio::transaction_cost(
            previous,
            current,
            parameters
        );

    assert(
        std::abs(cost - 100.0) < 1e-12
    );

    const double zero_cost =
        quant::portfolio::transaction_cost(
            previous,
            previous,
            parameters
        );

    assert(zero_cost == 0.0);

    const PairPosition next{
        -30000.0,
        20000.0
    };

    const TransactionCostParameters asymmetric{
        0.002,
        0.001
    };

    const double asymmetric_cost =
        quant::portfolio::transaction_cost(
            previous,
            next,
            asymmetric
        );

    assert(
        std::abs(
            asymmetric_cost - 80.0
        ) < 1e-12
    );

    bool threw = false;

    try {
        static_cast<void>(
            quant::portfolio::transaction_cost(
                previous,
                current,
                TransactionCostParameters{
                    -0.001,
                    0.001
                }
            )
        );
    }
    catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    return 0;
}