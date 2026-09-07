#include "quant/math/statistics.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::math {

double mean(const Series& series) {
    if (series.empty()) {
        throw std::invalid_argument(
            "Mean requires a non-empty series"
        );
    }

    double sum = 0.0;

    for (const auto& observation : series) {
        sum += observation.value;
    }

    return sum / static_cast<double>(series.size());
}

double variance(const Series& series) {
    if (series.size() < 2) {
        throw std::invalid_argument(
            "Variance requires at least two observations"
        );
    }

    const double mu = mean(series);

    double squared_deviations = 0.0;

    for (const auto& observation : series) {
        const double deviation =
            observation.value - mu;

        squared_deviations += deviation * deviation;
    }

    return squared_deviations /
           static_cast<double>(series.size() - 1);
}

double standard_deviation(const Series& series) {
    return std::sqrt(variance(series));
}

double covariance(
    const Series& x,
    const Series& y
) {
    if (x.size() != y.size()) {
        throw std::invalid_argument(
            "Covariance requires equal-length series"
        );
    }

    if (x.size() < 2) {
        throw std::invalid_argument(
            "Covariance requires at least two observations"
        );
    }

    for (std::size_t i = 0; i < x.size(); ++i) {
        if (x[i].timestamp != y[i].timestamp) {
            throw std::invalid_argument(
                "Covariance requires aligned timestamps"
            );
        }
    }

    const double mean_x = mean(x);
    const double mean_y = mean(y);

    double sum = 0.0;

    for (std::size_t i = 0; i < x.size(); ++i) {
        sum +=
            (x[i].value - mean_x) *
            (y[i].value - mean_y);
    }

    return sum /
           static_cast<double>(x.size() - 1);
}

double correlation(
    const Series& x,
    const Series& y
) {
    const double covariance_xy = covariance(x, y);

    const double std_x = standard_deviation(x);
    const double std_y = standard_deviation(y);

    if (std_x == 0.0 || std_y == 0.0) {
        throw std::domain_error(
            "Correlation undefined for zero-variance series"
        );
    }

    return covariance_xy / (std_x * std_y);
}

} // namespace quant::math