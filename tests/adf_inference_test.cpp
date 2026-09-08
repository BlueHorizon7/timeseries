#include "quant/math/adf_inference.hpp"

#include <cassert>
#include <cmath>

int main() {
    using quant::math::ADFDecision;
    using quant::math::ADFResult;
    using quant::math::DeterministicTerm;

    {
        const ADFResult result{
            -4.00,
            -0.50,
            0.125,
            1,
            DeterministicTerm::Intercept
        };

        const auto inference =
            quant::math::infer_adf(result);

        assert(
            inference.decision ==
            ADFDecision::RejectAtOnePercent
        );

        assert(
            std::abs(
                inference.critical_values.one_percent
                + 3.43
            ) < 1e-12
        );
    }

    {
        const ADFResult result{
            -3.00,
            -0.40,
            0.1333333333333333,
            1,
            DeterministicTerm::Intercept
        };

        const auto inference =
            quant::math::infer_adf(result);

        assert(
            inference.decision ==
            ADFDecision::RejectAtFivePercent
        );
    }

    {
        const ADFResult result{
            -2.70,
            -0.30,
            0.1111111111111111,
            1,
            DeterministicTerm::Intercept
        };

        const auto inference =
            quant::math::infer_adf(result);

        assert(
            inference.decision ==
            ADFDecision::RejectAtTenPercent
        );
    }

    {
        const ADFResult result{
            -2.00,
            -0.20,
            0.10,
            1,
            DeterministicTerm::Intercept
        };

        const auto inference =
            quant::math::infer_adf(result);

        assert(
            inference.decision ==
            ADFDecision::FailToReject
        );
    }

    return 0;
}