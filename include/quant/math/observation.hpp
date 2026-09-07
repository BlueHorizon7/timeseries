#pragma once

#include <cstdint>

namespace quant::math {

struct Observation {
    std::int64_t timestamp{};
    double value{};
};

} // namespace quant::math