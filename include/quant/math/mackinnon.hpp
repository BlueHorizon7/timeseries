#pragma once

#include <cstddef>

namespace quant::math {

struct MacKinnonCriticalValues {
    double one_percent{};
    double five_percent{};
    double ten_percent{};
};

[[nodiscard]]
MacKinnonCriticalValues
mackinnon_cointegration_critical_values(
    std::size_t observations
);

[[nodiscard]]
double
mackinnon_cointegration_p_value(
    double statistic,
    std::size_t observations
);

} // namespace quant::math