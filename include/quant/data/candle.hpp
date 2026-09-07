#pragma once

#include <cstdint>

namespace quant::data {
    struct Candle {
        std::int64_t timestamp{};
        double open{};
        double high{};
        double low{};
        double close{};
        double volume{};
    };
}