#include "quant/research/pair_selection.hpp"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace {

quant::math::Series make_base_series(
    std::size_t n
) {
    quant::math::Series result;
    result.reserve(n);

    double price = 100.0;

    for (std::size_t i = 0;
         i < n;
         ++i) {

        /*
         * Smooth integrated price process.
         *
         * The increments are positive but vary over time,
         * producing a realistic nonstationary price path.
         */
        const double increment =
            0.20 +
            0.05 *
                std::sin(
                    0.13 *
                    static_cast<double>(i)
                );

        price += increment;

        result.add(
            quant::math::Observation{
                static_cast<std::int64_t>(i),
                price
            }
        );
    }

    return result;
}

quant::math::Series make_cointegrated_series(
    const quant::math::Series& base
) {
    quant::math::Series result;
    result.reserve(base.size());

    /*
     * Explicit stationary AR(1) residual:
     *
     *     e_t = 0.25 e_{t-1} + u_t
     *
     * The innovations are deterministic but alternating,
     * which keeps the test completely reproducible.
     */
    double residual = 0.0;

    for (std::size_t i = 0;
         i < base.size();
         ++i) {

        const double innovation =
            (i % 2 == 0)
                ? 0.01
                : -0.01;

        residual =
            0.25 * residual +
            innovation;

        const double value =
            base[i].value +
            residual;

        result.add(
            quant::math::Observation{
                base[i].timestamp,
                value
            }
        );
    }

    return result;
}

quant::math::Series make_unrelated_series(
    std::size_t n
) {
    quant::math::Series result;
    result.reserve(n);

    double price = 100.0;

    for (std::size_t i = 0;
         i < n;
         ++i) {

        /*
         * Return process deliberately unrelated to AAA/BBB.
         */
        const double log_return =
            (i % 2 == 0)
                ? 0.01
                : -0.01;

        price *=
            std::exp(log_return);

        result.add(
            quant::math::Observation{
                static_cast<std::int64_t>(i),
                price
            }
        );
    }

    return result;
}

} // namespace

int main() {
    constexpr std::size_t observations = 250;

    const auto base =
        make_base_series(
            observations
        );

    const auto cointegrated =
        make_cointegrated_series(
            base
        );

    const auto unrelated =
        make_unrelated_series(
            observations
        );

    std::vector<
        quant::research::AssetSeries
    > universe;

    universe.push_back(
        quant::research::AssetSeries{
            "AAA",
            base
        }
    );

    universe.push_back(
        quant::research::AssetSeries{
            "BBB",
            cointegrated
        }
    );

    universe.push_back(
        quant::research::AssetSeries{
            "CCC",
            unrelated
        }
    );

    quant::research::PairSelectionParameters
        parameters;

    parameters.universe
        .minimum_absolute_correlation =
            0.70;

    parameters.universe
        .minimum_observations =
            30;

    parameters.universe
        .maximum_candidates =
            0;

    parameters.cointegration
        .automatic_lag_selection =
            false;

    parameters.cointegration
        .adf_lags =
            0;

    parameters.false_discovery_rate =
        0.05;

    const auto result =
        quant::research::select_pairs(
            universe,
            parameters
        );

    /*
     * Exactly one pair should survive the
     * correlation pre-screen:
     *
     *     AAA / BBB
     */
    assert(
        result.tested.size() == 1
    );

    const auto& entry =
        result.tested.front();

    assert(
        entry.candidate.x_index == 0
    );

    assert(
        entry.candidate.y_index == 1
    );

    /*
     * Correlation pre-screen must have passed.
     */
    assert(
        std::abs(
            entry.candidate.correlation
        ) >= 0.70
    );

    /*
     * MacKinnon p-value must be valid.
     */
    assert(
        entry.cointegration.p_value >=
        0.0
    );

    assert(
        entry.cointegration.p_value <=
        1.0
    );

    /*
     * The AR(1) stationary residual should produce
     * statistically significant evidence against
     * the no-cointegration null.
     */
    assert(
        entry.cointegration.p_value <
        0.05
    );

    /*
     * With exactly one tested hypothesis,
     * BH adjustment leaves its p-value unchanged.
     */
    assert(
        std::abs(
            entry.adjusted_p_value -
            entry.cointegration.p_value
        ) < 1e-12
    );

    assert(
        entry.adjusted_p_value <
        0.05
    );

    assert(
        entry.rejected
    );

    /*
     * The pair must therefore be selected.
     */
    assert(
        result.selected.size() == 1
    );

    assert(
        result.selected.front()
            .candidate.x_index == 0
    );

    assert(
        result.selected.front()
            .candidate.y_index == 1
    );

    return 0;
}