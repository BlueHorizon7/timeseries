#include "quant/data/validation.hpp"

#include <cassert>
#include <stdexcept>

int main() {
    using namespace quant::data;

    TimeSeries valid;

    valid.add(
        Candle{
            1000,
            10.0,
            12.0,
            9.0,
            11.0,
            1000.0
        }
    );

    validate_market_data(valid);

    TimeSeries invalid;

    invalid.add(
        Candle{
            1000,
            10.0,
            8.0,
            9.0,
            11.0,
            1000.0
        }
    );

    bool threw = false;

    try {
        validate_market_data(invalid);
    }
    catch (const std::domain_error&) {
        threw = true;
    }

    assert(threw);

    return 0;
}