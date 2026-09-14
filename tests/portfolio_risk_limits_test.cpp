#include "quant/risk/portfolio_risk_limits.hpp"

#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {

bool approximately_equal(
    double lhs,
    double rhs,
    double tolerance = 1e-12
) {
    return std::abs(lhs - rhs) < tolerance;
}

quant::risk::CovarianceMatrixResult
make_diagonal_covariance(
    const std::vector<double>& variances
) {
    quant::risk::CovarianceMatrixResult result;

    const std::size_t n =
        variances.size();

    result.covariance.assign(
        n,
        std::vector<double>(n, 0.0)
    );

    result.correlation.assign(
        n,
        std::vector<double>(n, 0.0)
    );

    for (std::size_t i = 0;
         i < n;
         ++i) {

        result.covariance[i][i] =
            variances[i];

        result.correlation[i][i] =
            1.0;
    }

    return result;
}

} // namespace

int main() {
    /*
     * No constraints bind.
     */
    {
        const auto covariance =
            make_diagonal_covariance(
                {1.0, 4.0}
            );

        const std::vector<double> weights{
            0.4,
            0.3
        };

        const quant::risk::PortfolioRiskLimits limits{
            1.0,
            0.0,
            0.0
        };

        const auto result =
            quant::risk::apply_portfolio_risk_limits(
                covariance,
                weights,
                limits
            );

        assert(result.input_weights == weights);
        assert(result.final_weights == weights);

        assert(
            approximately_equal(
                result.input_gross_exposure,
                0.7
            )
        );

        assert(
            approximately_equal(
                result.final_gross_exposure,
                0.7
            )
        );

        assert(!result.pair_weight_limited);
        assert(!result.gross_exposure_limited);
        assert(!result.volatility_limited);
        assert(
            approximately_equal(
                result.scale_factor,
                1.0
            )
        );
    }

    /*
     * Pair-weight limit.
     *
     * [0.8, 0.2] with max pair weight 0.5
     * becomes [0.5, 0.2].
     */
    {
        const auto covariance =
            make_diagonal_covariance(
                {1.0, 1.0}
            );

        const std::vector<double> weights{
            0.8,
            0.2
        };

        const quant::risk::PortfolioRiskLimits limits{
            2.0,
            0.5,
            0.0
        };

        const auto result =
            quant::risk::apply_portfolio_risk_limits(
                covariance,
                weights,
                limits
            );

        assert(
            approximately_equal(
                result.final_weights[0],
                0.5
            )
        );

        assert(
            approximately_equal(
                result.final_weights[1],
                0.2
            )
        );

        assert(result.pair_weight_limited);
    }

    /*
     * Gross-exposure scaling.
     *
     * [0.5, 0.5, 0.5]
     * gross = 1.5
     *
     * limit = 0.9
     *
     * factor = 0.9 / 1.5 = 0.6
     *
     * final = [0.3, 0.3, 0.3]
     */
    {
        const auto covariance =
            make_diagonal_covariance(
                {1.0, 1.0, 1.0}
            );

        const std::vector<double> weights{
            0.5,
            0.5,
            0.5
        };

        const quant::risk::PortfolioRiskLimits limits{
            0.9,
            0.0,
            0.0
        };

        const auto result =
            quant::risk::apply_portfolio_risk_limits(
                covariance,
                weights,
                limits
            );

        assert(result.gross_exposure_limited);

        assert(
            approximately_equal(
                result.scale_factor,
                0.6
            )
        );

        for (const double weight :
             result.final_weights) {

            assert(
                approximately_equal(
                    weight,
                    0.3
                )
            );
        }

        assert(
            approximately_equal(
                result.final_gross_exposure,
                0.9
            )
        );
    }

    /*
     * Volatility scaling.
     *
     * Sigma = diag(1,4)
     * w = [0.5,0.5]
     *
     * variance = 1.25
     * volatility = sqrt(1.25)
     *
     * target volatility = 0.5
     *
     * scale = 0.5 / sqrt(1.25)
     */
    {
        const auto covariance =
            make_diagonal_covariance(
                {1.0, 4.0}
            );

        const std::vector<double> weights{
            0.5,
            0.5
        };

        const quant::risk::PortfolioRiskLimits limits{
            10.0,
            0.0,
            0.5
        };

        const auto result =
            quant::risk::apply_portfolio_risk_limits(
                covariance,
                weights,
                limits
            );

        const double expected_scale =
            0.5 / std::sqrt(1.25);

        assert(result.volatility_limited);

        assert(
            approximately_equal(
                result.scale_factor,
                expected_scale
            )
        );

        assert(
            approximately_equal(
                result.final_portfolio_volatility,
                0.5
            )
        );
    }

    /*
     * Pair limit followed by gross limit.
     */
    {
        const auto covariance =
            make_diagonal_covariance(
                {1.0, 1.0}
            );

        const std::vector<double> weights{
            0.9,
            0.9
        };

        const quant::risk::PortfolioRiskLimits limits{
            0.8,
            0.6,
            0.0
        };

        const auto result =
            quant::risk::apply_portfolio_risk_limits(
                covariance,
                weights,
                limits
            );

        /*
         * Pair clipping:
         *
         * [0.9,0.9] -> [0.6,0.6]
         *
         * Gross = 1.2
         *
         * Gross scale:
         *
         * 0.8 / 1.2 = 2/3
         *
         * Final:
         *
         * [0.4,0.4]
         */
        assert(result.pair_weight_limited);
        assert(result.gross_exposure_limited);

        assert(
            approximately_equal(
                result.final_weights[0],
                0.4
            )
        );

        assert(
            approximately_equal(
                result.final_weights[1],
                0.4
            )
        );

        assert(
            approximately_equal(
                result.final_gross_exposure,
                0.8
            )
        );
    }

    /*
     * Zero gross-exposure limit forces the portfolio flat.
     */
    {
        const auto covariance =
            make_diagonal_covariance(
                {1.0, 1.0}
            );

        const quant::risk::PortfolioRiskLimits limits{
            0.0,
            0.0,
            0.0
        };

        const auto result =
            quant::risk::apply_portfolio_risk_limits(
                covariance,
                {0.5, -0.5},
                limits
            );

        assert(result.gross_exposure_limited);

        assert(
            approximately_equal(
                result.final_weights[0],
                0.0
            )
        );

        assert(
            approximately_equal(
                result.final_weights[1],
                0.0
            )
        );

        assert(
            approximately_equal(
                result.final_gross_exposure,
                0.0
            )
        );
    }

    /*
     * Invalid: wrong weight dimension.
     */
    {
        const auto covariance =
            make_diagonal_covariance(
                {1.0, 1.0}
            );

        bool rejected = false;

        try {
            static_cast<void>(
                quant::risk::apply_portfolio_risk_limits(
                    covariance,
                    {0.5},
                    {}
                )
            );
        }
        catch (const std::invalid_argument&) {
            rejected = true;
        }

        assert(rejected);
    }

    /*
     * Invalid: negative gross limit.
     */
    {
        bool rejected = false;

        const quant::risk::PortfolioRiskLimits limits{
            -1.0,
            0.0,
            0.0
        };

        try {
            static_cast<void>(
                quant::risk::apply_portfolio_risk_limits(
                    make_diagonal_covariance({1.0}),
                    {1.0},
                    limits
                )
            );
        }
        catch (const std::invalid_argument&) {
            rejected = true;
        }

        assert(rejected);
    }

    /*
     * Invalid: non-finite pair limit.
     */
    {
        bool rejected = false;

        const quant::risk::PortfolioRiskLimits limits{
            1.0,
            std::numeric_limits<double>::infinity(),
            0.0
        };

        try {
            static_cast<void>(
                quant::risk::apply_portfolio_risk_limits(
                    make_diagonal_covariance({1.0}),
                    {1.0},
                    limits
                )
            );
        }
        catch (const std::invalid_argument&) {
            rejected = true;
        }

        assert(rejected);
    }

    return 0;
}