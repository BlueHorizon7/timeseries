#include "quant/execution/fill.hpp"

#include <cassert>
#include <iostream>

int main() {
    using namespace quant::execution;

    Fill fill{};

    fill.order_id = 1;
    fill.symbol = "TEST";
    fill.side = OrderSide::Buy;
    fill.quantity = 25.0;
    fill.price = 101.5;
    fill.timestamp = 100;
    fill.commission = 0.25;

    validate_fill(fill);

    {
        Fill invalid = fill;
        invalid.quantity = 0.0;

        bool threw = false;

        try {
            validate_fill(invalid);
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    {
        Fill invalid = fill;
        invalid.price = -1.0;

        bool threw = false;

        try {
            validate_fill(invalid);
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    {
        Fill invalid = fill;
        invalid.commission = -0.1;

        bool threw = false;

        try {
            validate_fill(invalid);
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    {
        Fill invalid = fill;
        invalid.order_id = 0;

        bool threw = false;

        try {
            validate_fill(invalid);
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    std::cout
        << "Fill tests passed!\n";

    return 0;
}