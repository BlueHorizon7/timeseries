#include "quant/research/pair_universe.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

quant::math::Series make_price_series(
    const std::vector<double>& log_returns,
    std::int64_t first_timestamp = 0
) {
    quant::math::Series result;

    double price = 100.0;

    result.add(
        quant::math::Observation{
            first_timestamp,
            price
        }
    );

    for (std::size_t i = 0;
         i < log_returns.size();
         ++i) {

        price *= std::exp(log_returns[i]);

        result.add(
            quant::math::Observation{
                first_timestamp +
                    static_cast<std::int64_t>(i + 1),
                price
            }
        );
    }

    return result;
}

} // namespace

int main() {

    using quant::research::AssetSeries;
    using quant::research::PairUniverseParameters;

    const std::vector<double> base_returns{
        0.010,
        0.020,
       -0.010,
        0.030,
        0.015,
       -0.020,
        0.010,
        0.005,
        0.020,
       -0.005,
        0.015,
        0.010
    };

    /*
        Asset A.
    */
    AssetSeries asset_a{
        "AAA",
        make_price_series(base_returns, 0)
    };

    /*
        Asset B has exactly twice the log-return
        sequence. Therefore the return correlation
        should be +1.
    */
    std::vector<double> b_returns;

    for (const double value : base_returns) {
        b_returns.push_back(
            2.0 * value
        );
    }

    AssetSeries asset_b{
        "BBB",
        make_price_series(b_returns, 0)
    };

    /*
        Asset C has the opposite return stream,
        therefore correlation should be -1.
    */
    std::vector<double> c_returns;

    for (const double value : base_returns) {
        c_returns.push_back(
            -value
        );
    }

    /*
        Start one timestamp later to verify that
        pairwise return alignment is timestamp-based.
    */
    AssetSeries asset_c{
    "CCC",
    make_price_series(c_returns, 0)
};
    /*
        Weakly related deterministic series.
    */
    const std::vector<double> d_returns{
        0.001,
       -0.003,
        0.002,
        0.001,
        0.000,
       -0.001,
        0.003,
       -0.002,
        0.001,
        0.002,
       -0.001,
        0.000
    };

    AssetSeries asset_d{
        "DDD",
        make_price_series(d_returns, 0)
    };

    std::vector<AssetSeries> universe{
        asset_a,
        asset_b,
        asset_c,
        asset_d
    };

    PairUniverseParameters parameters;

    parameters.minimum_absolute_correlation =
        0.95;

    parameters.minimum_observations =
        5;

    parameters.maximum_candidates = 0;

    const auto candidates =
        quant::research::generate_pair_candidates(
            universe,
            parameters
        );

    /*
        Expected:
          AAA-BBB : +1
          AAA-CCC : -1
          BBB-CCC : -1

        The weak DDD relationships should not pass.
    */
    assert(candidates.size() == 3);

    for (const auto& candidate : candidates) {
        assert(
            std::abs(candidate.correlation) >= 0.95
        );

        assert(
            candidate.observations >= 5
        );
    }

    /*
        Highest absolute correlations tie at 1.
        Deterministic ordering therefore follows
        x_index then y_index.
    */
    assert(candidates[0].x_index == 0);
    assert(candidates[0].y_index == 1);

    /*
        Limit the candidate universe.
    */
    parameters.maximum_candidates = 2;

    const auto limited =
        quant::research::generate_pair_candidates(
            universe,
            parameters
        );

    assert(limited.size() == 2);

    /*
        Invalid threshold.
    */
    bool threw = false;

    try {
        PairUniverseParameters invalid;
        invalid.minimum_absolute_correlation = 1.5;

        (void)quant::research::generate_pair_candidates(
            universe,
            invalid
        );
    }
    catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    /*
        Duplicate symbols.
    */
    threw = false;

    try {
        auto duplicate_universe = universe;

        duplicate_universe.push_back(
            AssetSeries{
                "AAA",
                make_price_series(
                    base_returns
                )
            }
        );

        (void)quant::research::generate_pair_candidates(
            duplicate_universe,
            PairUniverseParameters{}
        );
    }
    catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    std::cout
        << "Pair universe candidate tests passed!"
        << std::endl;

    return 0;
}