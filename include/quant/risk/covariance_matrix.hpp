#pragma once

#include "quant/math/series.hpp"

#include <cstddef>
#include <vector>

namespace quant::risk {

struct CovarianceMatrixResult {
    std::vector<double> mean_returns;

    // covariance[i][j] = Cov(return series i, return series j)
    std::vector<std::vector<double>> covariance;

    // correlation[i][j] = Corr(return series i, return series j)
    std::vector<std::vector<double>> correlation;

    std::size_t observations{};
};

[[nodiscard]]
CovarianceMatrixResult build_covariance_matrix(
    const std::vector<quant::math::Series>& return_series
);

[[nodiscard]]
double portfolio_variance(
    const CovarianceMatrixResult& result,
    const std::vector<double>& weights
);

[[nodiscard]]
double portfolio_volatility(
    const CovarianceMatrixResult& result,
    const std::vector<double>& weights
);

} // namespace quant::risk