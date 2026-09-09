#include "quant/strategy/pairs_signal.hpp"

#include <cassert>
#include <cmath>
#include <stdexcept>

int main() {
    using quant::math::Observation;
    using quant::math::Series;
    using quant::strategy::Position;
    using quant::strategy::SignalParameters;

    const SignalParameters parameters{
        2.0,
        0.5
    };

    /*
        Direct state-transition tests.
    */

    assert(
        quant::strategy::classify_signal(
            -2.1,
            Position::Flat,
            parameters
        ) == Position::LongSpread
    );

    assert(
        quant::strategy::classify_signal(
            2.1,
            Position::Flat,
            parameters
        ) == Position::ShortSpread
    );

    assert(
        quant::strategy::classify_signal(
            0.2,
            Position::LongSpread,
            parameters
        ) == Position::Flat
    );

    /*
        Between entry and exit thresholds,
        maintain the current position.
    */

    assert(
        quant::strategy::classify_signal(
            -1.5,
            Position::LongSpread,
            parameters
        ) == Position::LongSpread
    );

    assert(
        quant::strategy::classify_signal(
            1.5,
            Position::ShortSpread,
            parameters
        ) == Position::ShortSpread
    );

    /*
        Generate a complete signal sequence.
    */

    Series zscores;

    zscores.add(Observation{1000, 0.0});
    zscores.add(Observation{2000, -2.1});
    zscores.add(Observation{3000, -1.5});
    zscores.add(Observation{4000, -0.8});
    zscores.add(Observation{5000, -0.4});
    zscores.add(Observation{6000, 2.2});
    zscores.add(Observation{7000, 1.4});
    zscores.add(Observation{8000, 0.3});

    const Series signals =
        quant::strategy::generate_signals(
            zscores,
            parameters
        );

    assert(signals.size() == 8);

    assert(signals[0].value == 0.0);
    assert(signals[1].value == 1.0);
    assert(signals[2].value == 1.0);
    assert(signals[3].value == 1.0);
    assert(signals[4].value == 0.0);
    assert(signals[5].value == -1.0);
    assert(signals[6].value == -1.0);
    assert(signals[7].value == 0.0);

    /*
        Non-finite z-score must fail.
    */

    bool threw = false;

    try {
        static_cast<void>(
            quant::strategy::classify_signal(
                std::nan(""),
                Position::Flat,
                parameters
            )
        );
    }
    catch (const std::domain_error&) {
        threw = true;
    }

    assert(threw);

    return 0;
}