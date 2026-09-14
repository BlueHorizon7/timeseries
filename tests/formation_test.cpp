#include "quant/research/formation.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>

int main() {
    quant::data::AlignedSeries data;

    constexpr std::size_t n = 500;

    /*
        Deterministic pseudo-random generator.

        This gives us reproducible innovations while producing
        a genuine random-walk-like integrated X process.
    */
    std::uint64_t state =
        88172645463325252ULL;

    double random_walk = 100.0;
    double stationary_noise = 0.0;

    for (std::size_t i = 0;
         i < n;
         ++i) {

        /*
            Deterministic uniform innovation in approximately
            [-1, 1].
        */
        state =
            state *
                2862933555777941757ULL +
            3037000493ULL;

        const double uniform =
            static_cast<double>(
                state >> 11
            ) *
            (1.0 / 9007199254740992.0);

        const double innovation =
            2.0 * uniform - 1.0;

        /*
            Integrated process:

                X_t = X_(t-1) + u_t
        */
        random_walk += innovation;

        /*
            Stationary AR(1)-like disturbance:

                e_t = 0.5 e_(t-1) + small innovation
        */
        stationary_noise =
            0.5 * stationary_noise +
            0.05 * innovation;

        /*
            True cointegrating relationship:

                Y_t = 5 + 2 X_t + e_t
        */
        const double x_value =
            random_walk;

        const double y_value =
            5.0 +
            2.0 * x_value +
            stationary_noise;

        data.add(
            quant::data::AlignedObservation{
                static_cast<std::int64_t>(i + 1),
                x_value,
                y_value
            }
        );
    }

    constexpr std::size_t formation_size = 350;

    quant::research::FormationParameters parameters;

    parameters.formation_size =
        formation_size;

    parameters.cointegration
        .automatic_lag_selection = true;

    parameters.cointegration
        .max_adf_lags = 8;

    parameters.cointegration
        .information_criterion =
            quant::math::InformationCriterion::AIC;

    const auto result =
        quant::research::run_formation_test(
            data,
            parameters
        );

    assert(
        result.formation_data.size() ==
        formation_size
    );

    assert(
        result.trading_data.size() ==
        n - formation_size
    );

    assert(
        result.formation_data[0].timestamp ==
        data[0].timestamp
    );

    assert(
        result.formation_data[
            formation_size - 1
        ].timestamp ==
        data[
            formation_size - 1
        ].timestamp
    );

    assert(
        result.trading_data[0].timestamp ==
        data[formation_size].timestamp
    );

    /*
        Automatic lag selection must choose a lag
        inside the requested range.
    */
    assert(
        result.cointegration.adf_lags <= 8
    );

    /*
        The synthetic pair is cointegrated by construction.
    */
    assert(
        result.passes
    );

    assert(
        result.cointegration.adf_statistic <
        result.cointegration
            .critical_values
            .five_percent
    );

    /*
        True hedge ratio is beta = 2.
    */
    assert(
        std::abs(
            result.cointegration
                .regression
                .slope -
            2.0
        ) < 0.05
    );

    return 0;
}