#include "quant/risk/portfolio_risk_limits.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace quant::risk {

namespace {

double gross_exposure(
    const std::vector<double>& weights
) {
    double result = 0.0;

    for (const double weight : weights) {
        result += std::abs(weight);
    }

    if (!std::isfinite(result)) {
        throw std::domain_error(
            "Gross exposure is non-finite"
        );
    }

    return result;
}

void validate_limits(
    const PortfolioRiskLimits& limits
) {
    if (!std::isfinite(
            limits.maximum_gross_exposure
        ) ||
        limits.maximum_gross_exposure < 0.0) {

        throw std::invalid_argument(
            "Maximum gross exposure must be "
            "finite and non-negative"
        );
    }

    if (!std::isfinite(
            limits.maximum_pair_weight
        ) ||
        limits.maximum_pair_weight < 0.0) {

        throw std::invalid_argument(
            "Maximum pair weight must be "
            "finite and non-negative"
        );
    }

    if (!std::isfinite(
            limits.maximum_portfolio_volatility
        ) ||
        limits.maximum_portfolio_volatility < 0.0) {

        throw std::invalid_argument(
            "Maximum portfolio volatility must be "
            "finite and non-negative"
        );
    }
}

void validate_covariance_dimensions(
    const CovarianceMatrixResult& covariance,
    std::size_t number_of_weights
) {
    const std::size_t n =
        covariance.covariance.size();

    if (n == 0) {
        throw std::invalid_argument(
            "Covariance matrix must not be empty"
        );
    }

    if (number_of_weights != n) {
        throw std::invalid_argument(
            "Portfolio weights must match "
            "covariance matrix dimension"
        );
    }

    for (const auto& row :
         covariance.covariance) {

        if (row.size() != n) {
            throw std::invalid_argument(
                "Covariance matrix must be square"
            );
        }

        for (const double value : row) {
            if (!std::isfinite(value)) {
                throw std::invalid_argument(
                    "Covariance matrix must contain "
                    "only finite values"
                );
            }
        }
    }
}

} // namespace

PortfolioRiskResult apply_portfolio_risk_limits(
    const CovarianceMatrixResult& covariance,
    const std::vector<double>& weights,
    const PortfolioRiskLimits& limits
) {
    validate_limits(limits);

    validate_covariance_dimensions(
        covariance,
        weights.size()
    );

    for (const double weight : weights) {
        if (!std::isfinite(weight)) {
            throw std::invalid_argument(
                "Portfolio weights must be finite"
            );
        }
    }

    PortfolioRiskResult result;

    result.input_weights = weights;
    result.final_weights = weights;

    result.input_gross_exposure =
        gross_exposure(weights);

    result.input_portfolio_variance =
        portfolio_variance(
            covariance,
            weights
        );

    result.input_portfolio_volatility =
        std::sqrt(
            result.input_portfolio_variance
        );

    /*
     * ---------------------------------------------------------
     * 1. Individual pair-weight limits
     * ---------------------------------------------------------
     *
     * maximum_pair_weight == 0 disables
     * this constraint.
     */
    if (limits.maximum_pair_weight > 0.0) {
        for (double& weight :
             result.final_weights) {

            const double absolute_weight =
                std::abs(weight);

            if (absolute_weight >
                limits.maximum_pair_weight) {

                weight =
                    std::copysign(
                        limits.maximum_pair_weight,
                        weight
                    );

                result.pair_weight_limited =
                    true;
            }
        }
    }

    /*
     * ---------------------------------------------------------
     * 2. Gross-exposure limit
     * ---------------------------------------------------------
     *
     * maximum_gross_exposure == 0 is a valid
     * hard zero-exposure limit.
     */
    const double current_gross =
        gross_exposure(
            result.final_weights
        );

    if (current_gross >
        limits.maximum_gross_exposure) {

        if (limits.maximum_gross_exposure == 0.0) {
            std::fill(
                result.final_weights.begin(),
                result.final_weights.end(),
                0.0
            );

            result.scale_factor = 0.0;
        } else {
            const double factor =
                limits.maximum_gross_exposure /
                current_gross;

            for (double& weight :
                 result.final_weights) {

                weight *= factor;
            }

            result.scale_factor *= factor;
        }

        result.gross_exposure_limited =
            true;
    }

    /*
     * ---------------------------------------------------------
     * 3. Portfolio-volatility limit
     * ---------------------------------------------------------
     *
     * maximum_portfolio_volatility == 0
     * DISABLES this constraint.
     *
     * For positive limits:
     *
     *     sigma(cw) = |c| sigma(w)
     *
     * so a single proportional scale factor
     * is sufficient.
     */
    if (limits.maximum_portfolio_volatility > 0.0) {
        const double current_variance =
            portfolio_variance(
                covariance,
                result.final_weights
            );

        const double current_volatility =
            std::sqrt(current_variance);

        if (current_volatility >
            limits.maximum_portfolio_volatility) {

            if (current_volatility > 0.0) {
                const double factor =
                    limits.maximum_portfolio_volatility /
                    current_volatility;

                for (double& weight :
                     result.final_weights) {

                    weight *= factor;
                }

                result.scale_factor *= factor;

                result.volatility_limited =
                    true;
            }
        }
    }

    result.final_gross_exposure =
        gross_exposure(
            result.final_weights
        );

    result.final_portfolio_variance =
        portfolio_variance(
            covariance,
            result.final_weights
        );

    result.final_portfolio_volatility =
        std::sqrt(
            result.final_portfolio_variance
        );

    return result;
}

} // namespace quant::risk