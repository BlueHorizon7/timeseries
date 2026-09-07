#include "quant/data/candle.hpp"

#include <cassert>

int main() {
    using quant::data::Candle;

    Candle candle{
        1'000,
        100.0,
        105.0,
        99.0,
        103.0,
        12'500.0
    };

    assert(candle.timestamp == 1'000);
    assert(candle.open == 100.0);
    assert(candle.high == 105.0);
    assert(candle.low == 99.0);
    assert(candle.close == 103.0);
    assert(candle.volume == 12'500.0);
}