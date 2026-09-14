#include "quant/risk/covariance_matrix.hpp"

#include "quant/math/statistics.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::risk {

CovarianceMatrixResult build_covariance_matrix(
    const std::vector<quant::math::Series>& return_series
) {
    if (return_series.empty()) {
        throw std::invalid_argument(
            "Covariance matrix requires at least one series"
        );
    }

    for (const auto& series : return_series) {
        if (series.size() < 2) {
            throw std::invalid_argument(
                "Every return series must contain at least "
                "two observations"
            );
        }

        for (std::size_t i = 1;
             i < series.size();
             ++i) {

            if (series[i].timestamp <=
                series[i - 1].timestamp) {

                throw std::invalid_argument(
                    "Return timestamps must be strictly increasing"
                );
            }
        }
    }

    const std::size_t n =
        return_series.size();

    CovarianceMatrixResult result;

    result.covariance.assign(
        n,
        std::vector<double>(n, 0.0)
    );

    result.correlation.assign(
        n,
        std::vector<double>(n, 0.0)
    );

    result.mean_returns.assign(
        n,
        0.0
    );

    /*
     * Covariance/correlation in quant::math require
     * timestamp-aligned series. Therefore every pairwise
     * calculation uses the existing authoritative
     * statistical implementation.
     */
    for (std::size_t i = 0;
         i < n;
         ++i) {

        if (return_series[i].size() < 2) {
            throw std::invalid_argument(
                "Insufficient observations"
            );
        }

        long double sum = 0.0L;

        for (const auto& observation :
             return_series[i]) {

            sum +=
                static_cast<long double>(
                    observation.value
                );
        }

        result.mean_returns[i] =
            static_cast<double>(
                sum /
                static_cast<long double>(
                    return_series[i].size()
                )
            );
    }

    for (std::size_t i = 0;
         i < n;
         ++i) {

        for (std::size_t j = i;
             j < n;
             ++j) {

            const double covariance =
                quant::math::covariance(
                    return_series[i],
                    return_series[j]
                );

            if (!std::isfinite(covariance)) {
                throw std::domain_error(
                    "Covariance matrix contains "
                    "a non-finite value"
                );
            }

            result.covariance[i][j] =
                covariance;

            result.covariance[j][i] =
                covariance;

            if (i == j) {
                result.correlation[i][j] = 1.0;
            } else {
                const double correlation =
                    quant::math::correlation(
                        return_series[i],
                        return_series[j]
                    );

                if (!std::isfinite(correlation)) {
                    throw std::domain_error(
                        "Correlation matrix contains "
                        "a non-finite value"
                    );
                }

                result.correlation[i][j] =
                    correlation;

                result.correlation[j][i] =
                    correlation;
            }
        }
    }

    result.observations =
        return_series.front().size();

    /*
     * All pairwise operations require aligned
     * timestamps. Thus all series must have the
     * same effective observation count here.
     */
    return result;
}

double portfolio_variance(
    const CovarianceMatrixResult& result,
    const std::vector<double>& weights
) {
    const std::size_t n =
        result.covariance.size();

    if (n == 0) {
        throw std::invalid_argument(
            "Portfolio variance requires a "
            "non-empty covariance matrix"
        );
    }

    if (weights.size() != n) {
        throw std::invalid_argument(
            "Portfolio weights must match "
            "covariance matrix dimension"
        );
    }

    long double variance = 0.0L;

    for (const double weight : weights) {
        if (!std::isfinite(weight)) {
            throw std::invalid_argument(
                "Portfolio weights must be finite"
            );
        }
    }

    for (std::size_t i = 0;
         i < n;
         ++i) {

        for (std::size_t j = 0;
             j < n;
             ++j) {

            variance +=
                static_cast<long double>(
                    weights[i]
                ) *
                static_cast<long double>(
                    result.covariance[i][j]
                ) *
                static_cast<long double>(
                    weights[j]
                );
        }
    }

    if (!std::isfinite(
            static_cast<double>(variance)
        )) {

        throw std::domain_error(
            "Portfolio variance is non-finite"
        );
    }

    if (variance < 0.0L) {
        if (variance > -1e-15L) {
            variance = 0.0L;
        } else {
            throw std::domain_error(
                "Portfolio variance is negative"
            );
        }
    }

    return static_cast<double>(variance);
}

double portfolio_volatility(
    const CovarianceMatrixResult& result,
    const std::vector<double>& weights
) {
    return std::sqrt(
        portfolio_variance(
            result,
            weights
        )
    );
}

} // namespace quant::risk