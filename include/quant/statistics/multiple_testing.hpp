#pragma once

#include <cstddef>
#include <vector>

namespace quant::statistics {

struct MultipleTestingResult {
    std::vector<double> adjusted_p_values;
    std::vector<bool> rejected;

    std::size_t number_rejected{};
};

[[nodiscard]]
MultipleTestingResult benjamini_hochberg(
    const std::vector<double>& p_values,
    double false_discovery_rate
);

} // namespace quant::statistics