#include "quant/execution/portfolio_ledger.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <unordered_map>

int main() {
    using namespace quant::execution;

    PortfolioLedger ledger{};

    ledger.cash = 100000.0;

    apply_fill(
        ledger,
        Fill{
            1,
            "AAPL",
            OrderSide::Buy,
            100.0,
            100.0,
            1,
            10.0
        }
    );

    assert(
        std::abs(
            ledger.cash -
            89990.0
        ) < 1e-12
    );

    const auto& apple =
        position(
            ledger,
            "AAPL"
        );

    assert(
        apple.quantity == 100.0
    );

    assert(
        apple.average_entry_price == 100.0
    );

    std::unordered_map<std::string, double>
        prices{
            {"AAPL", 110.0}
        };

    mark_to_market(
        ledger,
        prices,
        2
    );

    assert(
        std::abs(
            ledger.unrealized_pnl -
            1000.0
        ) < 1e-12
    );

    assert(
        std::abs(
            equity(
                ledger,
                prices
            ) -
            100990.0
        ) < 1e-12
    );

    /*
        Partial closing trade.
    */
    apply_fill(
        ledger,
        Fill{
            2,
            "AAPL",
            OrderSide::Sell,
            40.0,
            120.0,
            3,
            4.0
        }
    );

    assert(
        position(
            ledger,
            "AAPL"
        ).quantity == 60.0
    );

    assert(
        std::abs(
            ledger.cash -
            94786.0
        ) < 1e-12
    );

    /*
        Portfolio positions should be deterministic.
    */
    apply_fill(
        ledger,
        Fill{
            3,
            "MSFT",
            OrderSide::Buy,
            10.0,
            200.0,
            4,
            1.0
        }
    );

    const auto all_positions =
        positions(ledger);

    assert(
        all_positions.size() == 2
    );

    assert(
        all_positions[0].symbol ==
        "AAPL"
    );

    assert(
        all_positions[1].symbol ==
        "MSFT"
    );

    /*
        Missing mark price must fail.
    */
    {
        bool threw = false;

        try {
            mark_to_market(
                ledger,
                std::unordered_map<std::string, double>{
                    {"AAPL", 110.0}
                },
                5
            );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    /*
        Unknown position must fail.
    */
    {
        bool threw = false;

        try {
            (void)position(
                ledger,
                "UNKNOWN"
            );
        } catch (...) {
            threw = true;
        }

        assert(threw);
    }

    std::cout
        << "Portfolio ledger tests passed!\n";

    return 0;
}