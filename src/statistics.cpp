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

    double result = 0.0;

    for (std::size_t i = 0; i < series.size(); ++i) {
        const double value =
            series[i].value;

        if (!std::isfinite(value)) {
            throw std::domain_error(
                "Mean requires finite observations"
            );
        }

        const double k =
            static_cast<double>(i + 1);

        result +=
            (value - result) / k;
    }

    return result;
}

double variance(const Series& series) {
    if (series.size() < 2) {
        throw std::invalid_argument(
            "Variance requires at least two observations"
        );
    }

    double mean_value = 0.0;
    double sum_squared_deviations = 0.0;

    for (std::size_t i = 0; i < series.size(); ++i) {
        const double value =
            series[i].value;

        if (!std::isfinite(value)) {
            throw std::domain_error(
                "Variance requires finite observations"
            );
        }

        const double n =
            static_cast<double>(i + 1);

        const double delta =
            value - mean_value;

        mean_value +=
            delta / n;

        const double delta_after =
            value - mean_value;

        sum_squared_deviations +=
            delta * delta_after;
    }

    const double result =
        sum_squared_deviations /
        static_cast<double>(series.size() - 1);

    if (!std::isfinite(result)) {
        throw std::overflow_error(
            "Variance calculation overflow"
        );
    }

    return result;
}

double standard_deviation(const Series& series) {
    return std::sqrt(
        variance(series)
    );
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

    double mean_x = 0.0;
    double mean_y = 0.0;
    double covariance_sum = 0.0;

    for (std::size_t i = 0; i < x.size(); ++i) {
        if (x[i].timestamp != y[i].timestamp) {
            throw std::invalid_argument(
                "Covariance requires aligned timestamps"
            );
        }

        const double value_x =
            x[i].value;

        const double value_y =
            y[i].value;

        if (!std::isfinite(value_x) ||
            !std::isfinite(value_y)) {
            throw std::domain_error(
                "Covariance requires finite observations"
            );
        }

        const double n =
            static_cast<double>(i + 1);

        const double delta_x =
            value_x - mean_x;

        const double delta_y =
            value_y - mean_y;

        mean_x +=
            delta_x / n;

        mean_y +=
            delta_y / n;

        covariance_sum +=
            delta_x *
            (value_y - mean_y);
    }

    return covariance_sum /
           static_cast<double>(x.size() - 1);
}

double correlation(
    const Series& x,
    const Series& y
) {
    const double covariance_xy =
        covariance(x, y);

    const double std_x =
        standard_deviation(x);

    const double std_y =
        standard_deviation(y);

    if (std_x == 0.0 ||
        std_y == 0.0) {
        throw std::domain_error(
            "Correlation undefined for zero-variance series"
        );
    }

    return covariance_xy /
           (std_x * std_y);
}

} // namespace quant::math