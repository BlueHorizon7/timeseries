#include "quant/data/time_series.hpp"
#include "quant/math/returns.hpp"

#include <cassert>
#include <cmath>

int main() {
    using quant::data::Candle;
    using quant::data::TimeSeries;
    using quant::math::log_returns;

    TimeSeries prices;

    prices.add(Candle{
        1000,
        100.0,
        100.0,
        100.0,
        100.0,
        1000.0
    });

    prices.add(Candle{
        2000,
        110.0,
        110.0,
        110.0,
        110.0,
        1000.0
    });

    prices.add(Candle{
        3000,
        99.0,
        99.0,
        99.0,
        99.0,
        1000.0
    });

    const auto returns = log_returns(prices);

    assert(returns.size() == 2);

    const double expected_1 = std::log(110.0 / 100.0);
    const double expected_2 = std::log(99.0 / 110.0);

    assert(std::abs(returns[0].value - expected_1) < 1e-12);
    assert(std::abs(returns[1].value - expected_2) < 1e-12);

    assert(returns[0].timestamp == 2000);
    assert(returns[1].timestamp == 3000);
}