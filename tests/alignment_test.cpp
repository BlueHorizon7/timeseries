#include "quant/data/alignment.hpp"

#include <cassert>
#include <cmath>

int main() {
    using namespace quant::data;

    TimeSeries x;

    x.add(Candle{
        1000,
        10.0,
        11.0,
        9.0,
        10.5,
        1000.0
    });

    x.add(Candle{
        2000,
        11.0,
        12.0,
        10.0,
        11.5,
        1100.0
    });

    x.add(Candle{
        3000,
        12.0,
        13.0,
        11.0,
        12.5,
        1200.0
    });

    x.add(Candle{
        4000,
        13.0,
        14.0,
        12.0,
        13.5,
        1300.0
    });

    TimeSeries y;

    y.add(Candle{
        1000,
        20.0,
        21.0,
        19.0,
        20.5,
        2000.0
    });

    y.add(Candle{
        3000,
        22.0,
        23.0,
        21.0,
        22.5,
        2200.0
    });

    y.add(Candle{
        4000,
        23.0,
        24.0,
        22.0,
        23.5,
        2300.0
    });

    const auto aligned =
        inner_join(x, y);

    /*
     * Timestamp 2000 exists only in x,
     * so it must be excluded.
     */
    assert(aligned.size() == 3);

    assert(aligned[0].timestamp == 1000);
    assert(aligned[1].timestamp == 3000);
    assert(aligned[2].timestamp == 4000);

    assert(std::abs(
        aligned[0].x - 10.5
    ) < 1e-12);

    assert(std::abs(
        aligned[0].y - 20.5
    ) < 1e-12);

    assert(std::abs(
        aligned[1].x - 12.5
    ) < 1e-12);

    assert(std::abs(
        aligned[1].y - 22.5
    ) < 1e-12);

    return 0;
}