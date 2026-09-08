#include "quant/math/adf.hpp"

#include <cmath>
#include <stdexcept>
#include <vector>

namespace quant::math {

namespace {

struct RegressionResult {
    double coefficient{};
    double standard_error{};
};

RegressionResult
regress_gamma(
    const std::vector<double>& y,
    const std::vector<double>& x_gamma,
    const std::vector<std::vector<double>>& controls
) {
    const std::size_t n = y.size();
    const std::size_t k = 1 + controls.size();

    if (n <= k) {
        throw std::invalid_argument(
            "ADF regression has insufficient observations"
        );
    }

    /*
        Design matrix columns:

            x_gamma
            controls...

        We solve:

            beta = (X'X)^(-1) X'y

        using Gaussian elimination.
    */

    std::vector<std::vector<double>> matrix(
        k,
        std::vector<double>(k + 1, 0.0)
    );

    for (std::size_t i = 0; i < n; ++i) {
        std::vector<double> row;
        row.reserve(k);

        row.push_back(x_gamma[i]);

        for (const auto& control : controls) {
            row.push_back(control[i]);
        }

        for (std::size_t r = 0; r < k; ++r) {
            for (std::size_t c = 0; c < k; ++c) {
                matrix[r][c] += row[r] * row[c];
            }

            matrix[r][k] += row[r] * y[i];
        }
    }

    // Gaussian elimination with partial pivoting.

    for (std::size_t col = 0; col < k; ++col) {
        std::size_t pivot = col;

        for (std::size_t row = col + 1; row < k; ++row) {
            if (std::abs(matrix[row][col]) >
                std::abs(matrix[pivot][col])) {
                pivot = row;
            }
        }

        if (std::abs(matrix[pivot][col]) < 1e-14) {
            throw std::domain_error(
                "ADF regression design matrix is singular"
            );
        }

        std::swap(matrix[col], matrix[pivot]);

        for (std::size_t row = col + 1; row < k; ++row) {
            const double factor =
                matrix[row][col] / matrix[col][col];

            for (std::size_t c = col; c <= k; ++c) {
                matrix[row][c] -=
                    factor * matrix[col][c];
            }
        }
    }

    std::vector<double> coefficients(k);

    for (std::size_t i = k; i-- > 0;) {
        double value = matrix[i][k];

        for (std::size_t j = i + 1; j < k; ++j) {
            value -=
                matrix[i][j] * coefficients[j];
        }

        coefficients[i] =
            value / matrix[i][i];
    }

    // Calculate residual sum of squares.

    double rss = 0.0;

    for (std::size_t i = 0; i < n; ++i) {
        std::vector<double> row;
        row.reserve(k);

        row.push_back(x_gamma[i]);

        for (const auto& control : controls) {
            row.push_back(control[i]);
        }

        double fitted = 0.0;

        for (std::size_t j = 0; j < k; ++j) {
            fitted += coefficients[j] * row[j];
        }

        const double residual =
            y[i] - fitted;

        rss += residual * residual;
    }

    const double sigma_squared =
        rss / static_cast<double>(n - k);

    /*
        We need the (0,0) element of (X'X)^(-1).
        Solve X'X z = e_0.
    */

    std::vector<std::vector<double>> inverse_system(
        k,
        std::vector<double>(k + 1, 0.0)
    );

    for (std::size_t r = 0; r < k; ++r) {
        for (std::size_t c = 0; c < k; ++c) {
            inverse_system[r][c] =
                matrix[r][c];
        }

        inverse_system[r][k] =
            r == 0 ? 1.0 : 0.0;
    }

    for (std::size_t col = 0; col < k; ++col) {
        std::size_t pivot = col;

        for (std::size_t row = col + 1; row < k; ++row) {
            if (std::abs(inverse_system[row][col]) >
                std::abs(inverse_system[pivot][col])) {
                pivot = row;
            }
        }

        std::swap(
            inverse_system[col],
            inverse_system[pivot]
        );

        const double divisor =
            inverse_system[col][col];

        if (std::abs(divisor) < 1e-14) {
            throw std::domain_error(
                "ADF covariance matrix is singular"
            );
        }

        for (std::size_t c = col; c <= k; ++c) {
            inverse_system[col][c] /= divisor;
        }

        for (std::size_t row = 0; row < k; ++row) {
            if (row == col) {
                continue;
            }

            const double factor =
                inverse_system[row][col];

            for (std::size_t c = col; c <= k; ++c) {
                inverse_system[row][c] -=
                    factor * inverse_system[col][c];
            }
        }
    }

    const double variance_gamma =
        sigma_squared *
        inverse_system[0][k];

    return RegressionResult{
        coefficients[0],
        std::sqrt(variance_gamma)
    };
}

} // namespace

ADFResult augmented_dickey_fuller(
    const Series& series,
    std::size_t lags,
    DeterministicTerm deterministic
) {
    if (series.size() < lags + 5) {
        throw std::invalid_argument(
            "ADF test requires more observations for requested lag count"
        );
    }

    /*
        Regression:

        ΔS_t =
            gamma S_(t-1)
            + deterministic terms
            + delta_1 ΔS_(t-1)
            + ...
            + delta_p ΔS_(t-p)
            + error_t
    */

    std::vector<double> y;
    std::vector<double> x_gamma;

    std::vector<std::vector<double>> controls;

    if (deterministic == DeterministicTerm::Intercept ||
        deterministic == DeterministicTerm::InterceptAndTrend) {
        controls.emplace_back();
    }

    if (deterministic == DeterministicTerm::InterceptAndTrend) {
        controls.emplace_back();
    }

    for (auto& control : controls) {
        control.reserve(
            series.size() - lags - 1
        );
    }

    std::vector<std::vector<double>> delta_lags;

    for (std::size_t lag = 1; lag <= lags; ++lag) {
        delta_lags.emplace_back();
        delta_lags.back().reserve(
            series.size() - lags - 1
        );
    }

    const std::size_t first =
        lags + 1;

    for (std::size_t t = first; t < series.size(); ++t) {
        const double delta =
            series[t].value -
            series[t - 1].value;

        y.push_back(delta);

        x_gamma.push_back(
            series[t - 1].value
        );

        std::size_t control_index = 0;

        if (deterministic == DeterministicTerm::Intercept ||
            deterministic == DeterministicTerm::InterceptAndTrend) {

            controls[control_index++].push_back(1.0);
        }

        if (deterministic == DeterministicTerm::InterceptAndTrend) {
            controls[control_index++].push_back(
                static_cast<double>(t)
            );
        }

        for (std::size_t lag = 1; lag <= lags; ++lag) {
            delta_lags[lag - 1].push_back(
                series[t - lag].value -
                series[t - lag - 1].value
            );
        }
    }

    for (auto& delta_lag : delta_lags) {
        controls.push_back(
            std::move(delta_lag)
        );
    }

    const RegressionResult result =
        regress_gamma(
            y,
            x_gamma,
            controls
        );

    return ADFResult{
        result.coefficient / result.standard_error,
        result.coefficient,
        result.standard_error,
        lags,
        deterministic
    };
}

} // namespace quant::math