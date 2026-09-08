#include "quant/math/unit_root.hpp"

#include <cassert>

int main() {
    using quant::math::UnitRootDecision;

    {
        const auto result =
            quant::math::classify_dickey_fuller(-3.50);

        assert(
            result.decision ==
            UnitRootDecision::RejectNull
        );

        assert(result.critical_value_5pct < 0.0);
    }

    {
        const auto result =
            quant::math::classify_dickey_fuller(-2.00);

        assert(
            result.decision ==
            UnitRootDecision::FailToRejectNull
        );
    }

    return 0;
}