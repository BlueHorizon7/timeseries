#pragma once

#include "quant/risk/covariance_matrix.hpp"

#include <cstddef>
#include <vector>

namespace quant::risk {

struct PortfolioRiskLimits {
    /*
     * Maximum sum of absolute portfolio weights.
     *
     * Example:
     *     weights = [0.4, 0.4, 0.2]
     *     gross   = 1.0
     */
    double maximum_gross_exposure{1.0};

    /*
     * Maximum absolute weight of an individual pair.
     *
     * Zero disables this constraint.
     */
    double maximum_pair_weight{};

    /*
     * Maximum portfolio volatility.
     *
     * Zero disables this constraint.
     */
    double maximum_portfolio_volatility{};
};

struct PortfolioRiskResult {
    std::vector<double> input_weights;
    std::vector<double> final_weights;

    double input_gross_exposure{};
    double final_gross_exposure{};

    double input_portfolio_variance{};
    double final_portfolio_variance{};

    double input_portfolio_volatility{};
    double final_portfolio_volatility{};

    /*
     * Product of all portfolio-wide scaling operations.
     *
     * Pair-level clipping is not represented by this value.
     */
    double scale_factor{1.0};

    bool pair_weight_limited{};
    bool gross_exposure_limited{};
    bool volatility_limited{};
};

[[nodiscard]]
PortfolioRiskResult apply_portfolio_risk_limits(
    const CovarianceMatrixResult& covariance,
    const std::vector<double>& weights,
    const PortfolioRiskLimits& limits
);

} // namespace quant::risk