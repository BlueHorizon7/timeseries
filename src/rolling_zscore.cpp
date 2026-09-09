#include "quant/math/rolling_zscore.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::math {

Series rolling_zscore(
    const Series& series,
    std::size_t window
) {
    if (window < 2) {
        throw std::invalid_argument(
            "Rolling z-score requires a window of at least two observations"
        );
    }

    if (series.size() <= window) {
        throw std::invalid_argument(
            "Rolling z-score requires observations beyond the estimation window"
        );
    }

    Series result;

    result.reserve(series.size() - window);

    /*
        At time t, calculate statistics using:

            [t - window, ..., t - 1]

        and evaluate the current observation:

            z_t = (S_t - mean_t) / std_t

        Therefore S_t itself does not influence
        the mean or standard deviation used to
        calculate z_t.
    */

    for (std::size_t t = window; t < series.size(); ++t) {
        double sum = 0.0;

        for (std::size_t i = t - window; i < t; ++i) {
            sum += series[i].value;
        }

        const double mean =
            sum / static_cast<double>(window);

        double squared_deviations = 0.0;

        for (std::size_t i = t - window; i < t; ++i) {
            const double deviation =
                series[i].value - mean;

            squared_deviations +=
                deviation * deviation;
        }

        const double variance =
            squared_deviations /
            static_cast<double>(window - 1);

        const double standard_deviation =
            std::sqrt(variance);

        if (standard_deviation == 0.0) {
            throw std::domain_error(
                "Rolling z-score undefined for zero-variance window"
            );
        }

        const double z_score =
            (series[t].value - mean) /
            standard_deviation;

        result.add(
            Observation{
                series[t].timestamp,
                z_score
            }
        );
    }

    return result;
}

} // namespace quant::math