#include "quant/risk/covariance_matrix.hpp"

#include "quant/math/observation.hpp"
#include "quant/math/series.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace {

quant::math::Series make_series(
    const std::vector<double>& values
) {
    quant::math::Series result;

    for (std::size_t i = 0; i < values.size(); ++i) {
        result.add(
            quant::math::Observation{
                static_cast<std::int64_t>(
                    1000 * (i + 1)
                ),
                values[i]
            }
        );
    }

    return result;
}

bool approximately_equal(
    double lhs,
    double rhs,
    double tolerance = 1e-12
) {
    return std::abs(lhs - rhs) < tolerance;
}

} // namespace

int main() {
    /*
     * Two aligned series:
     *
     * X = [1,2,3,4]
     * Y = [2,4,6,8]
     *
     * Var(X) = 5/3
     * Var(Y) = 20/3
     * Cov(X,Y) = 10/3
     * Corr(X,Y) = 1
     */
    {
        const auto x =
            make_series(
                {1.0, 2.0, 3.0, 4.0}
            );

        const auto y =
            make_series(
                {2.0, 4.0, 6.0, 8.0}
            );

        const auto result =
            quant::risk::build_covariance_matrix(
                {x, y}
            );

        assert(result.observations == 4);

        assert(result.covariance.size() == 2);
        assert(result.covariance[0].size() == 2);
        assert(result.covariance[1].size() == 2);

        assert(result.correlation.size() == 2);
        assert(result.correlation[0].size() == 2);
        assert(result.correlation[1].size() == 2);

        assert(
            approximately_equal(
                result.covariance[0][0],
                5.0 / 3.0
            )
        );

        assert(
            approximately_equal(
                result.covariance[1][1],
                20.0 / 3.0
            )
        );

        assert(
            approximately_equal(
                result.covariance[0][1],
                10.0 / 3.0
            )
        );

        assert(
            approximately_equal(
                result.covariance[1][0],
                10.0 / 3.0
            )
        );

        assert(
            approximately_equal(
                result.correlation[0][0],
                1.0
            )
        );

        assert(
            approximately_equal(
                result.correlation[1][1],
                1.0
            )
        );

        assert(
            approximately_equal(
                result.correlation[0][1],
                1.0
            )
        );

        assert(
            approximately_equal(
                result.correlation[1][0],
                1.0
            )
        );
    }

    /*
     * Three-series covariance matrix.
     */
    {
        const auto x =
            make_series(
                {1.0, 2.0, 3.0, 4.0}
            );

        const auto y =
            make_series(
                {2.0, 4.0, 6.0, 8.0}
            );

        const auto z =
            make_series(
                {4.0, 3.0, 2.0, 1.0}
            );

        const auto result =
            quant::risk::build_covariance_matrix(
                {x, y, z}
            );

        assert(result.observations == 4);

        assert(
            approximately_equal(
                result.covariance[0][2],
                -5.0 / 3.0
            )
        );

        assert(
            approximately_equal(
                result.covariance[2][0],
                -5.0 / 3.0
            )
        );

        assert(
            approximately_equal(
                result.covariance[1][2],
                -10.0 / 3.0
            )
        );

        assert(
            approximately_equal(
                result.covariance[2][1],
                -10.0 / 3.0
            )
        );

        assert(
            approximately_equal(
                result.correlation[0][2],
                -1.0
            )
        );

        assert(
            approximately_equal(
                result.correlation[1][2],
                -1.0
            )
        );
    }

    /*
     * Portfolio variance:
     *
     * Sigma =
     * [ 1 0 ]
     * [ 0 4 ]
     *
     * w = [0.5, 0.5]
     *
     * variance = 1.25
     */
    {
        quant::risk::CovarianceMatrixResult result;

        result.covariance = {
            {1.0, 0.0},
            {0.0, 4.0}
        };

        assert(
            approximately_equal(
                quant::risk::portfolio_variance(
                    result,
                    {0.5, 0.5}
                ),
                1.25
            )
        );

        assert(
            approximately_equal(
                quant::risk::portfolio_volatility(
                    result,
                    {0.5, 0.5}
                ),
                std::sqrt(1.25)
            )
        );
    }

    /*
     * Existing statistics invariant:
     * timestamp mismatch must be rejected.
     */
    {
        const auto x =
            make_series(
                {1.0, 2.0, 3.0}
            );

        quant::math::Series y;

        y.add(
            quant::math::Observation{
                1000,
                2.0
            }
        );

        y.add(
            quant::math::Observation{
                2001,
                4.0
            }
        );

        y.add(
            quant::math::Observation{
                3000,
                6.0
            }
        );

        bool rejected = false;

        try {
            static_cast<void>(
                quant::risk::build_covariance_matrix(
                    {x, y}
                )
            );
        }
        catch (const std::invalid_argument&) {
            rejected = true;
        }

        assert(rejected);
    }

    /*
     * Empty input.
     */
    {
        bool rejected = false;

        try {
            static_cast<void>(
                quant::risk::build_covariance_matrix(
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
     * Incorrect portfolio dimension.
     */
    {
        quant::risk::CovarianceMatrixResult result;

        result.covariance = {
            {1.0, 0.0},
            {0.0, 1.0}
        };

        bool rejected = false;

        try {
            static_cast<void>(
                quant::risk::portfolio_variance(
                    result,
                    {1.0}
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