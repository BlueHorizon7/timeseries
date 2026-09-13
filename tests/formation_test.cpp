#include "quant/research/formation.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>

int main() {
    quant::data::AlignedSeries data;

    constexpr std::size_t n = 100;

    for (std::size_t i = 0;
         i < n;
         ++i) {

        const double x =
            100.0 +
            static_cast<double>(i);

        const double y =
            5.0 +
            2.0 * x +
            0.1 *
            std::sin(
                static_cast<double>(i)
            );

        data.add(
            quant::data::AlignedObservation{
                static_cast<std::int64_t>(i + 1),
                x,
                y
            }
        );
    }

    constexpr std::size_t formation_size = 70;

    const auto result =
        quant::research::run_formation_test(
            data,
            formation_size
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

    assert(
        result.passes
    );

    assert(
        result.cointegration.adf_statistic <
        result.cointegration
            .critical_values
            .five_percent
    );

    return 0;
}