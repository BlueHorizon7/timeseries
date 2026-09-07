#include "quant/data/time_series.hpp"

#include <cassert>
#include <stdexcept>

int main() {
    using quant::data::Candle;
    using quant::data::TimeSeries;

    TimeSeries series;

    series.add(Candle{
        1000,
        100.0,
        105.0,
        99.0,
        103.0,
        1000.0
    });

    series.add(Candle{
        2000,
        103.0,
        107.0,
        102.0,
        106.0,
        1200.0
    });

    assert(series.size() == 2);
    assert(series[0].close == 103.0);
    assert(series[1].close == 106.0);

    bool rejected = false;

    try {
        series.add(Candle{
            1500,
            106.0,
            108.0,
            105.0,
            107.0,
            900.0
        });
    }
    catch (const std::invalid_argument&) {
        rejected = true;
    }

    assert(rejected);
}