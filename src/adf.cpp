#include "quant/math/adf.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

namespace quant::math {

namespace {

struct RegressionResult {
    double coefficient{};
    double standard_error{};
};

struct Matrix {
    std::size_t rows{};
    std::size_t cols{};
    std::vector<double> data;

    double& operator()(std::size_t r, std::size_t c) {
        return data[r * cols + c];
    }

    double operator()(std::size_t r, std::size_t c) const {
        return data[r * cols + c];
    }
};

RegressionResult regress_gamma(
    const std::vector<double>& y,
    const std::vector<double>& x_gamma,
    const std::vector<std::vector<double>>& controls
) {
    const std::size_t n = y.size();
    const std::size_t k = 1 + controls.size();

    if (x_gamma.size() != n) {
        throw std::invalid_argument(
            "ADF gamma regressor size mismatch"
        );
    }

    for (const auto& control : controls) {
        if (control.size() != n) {
            throw std::invalid_argument(
                "ADF control regressor size mismatch"
            );
        }
    }

    if (n <= k) {
        throw std::invalid_argument(
            "ADF regression has insufficient observations"
        );
    }

    /*
        Design matrix:

            column 0 = S_(t-1)
            remaining columns = deterministic terms
                              + lagged differences

        We solve:

            min ||X beta - y||_2

        using Householder QR.
    */

    Matrix a{
        n,
        k,
        std::vector<double>(n * k, 0.0)
    };

    for (std::size_t i = 0; i < n; ++i) {
        a(i, 0) = x_gamma[i];

        for (std::size_t j = 0; j < controls.size(); ++j) {
            a(i, j + 1) = controls[j][i];
        }
    }

    std::vector<double> transformed_y = y;

    /*
        Householder QR.

        At step j we eliminate entries below A(j,j).
    */
    for (std::size_t j = 0; j < k; ++j) {

        long double norm_squared = 0.0L;

        for (std::size_t i = j; i < n; ++i) {
            const long double value = a(i, j);
            norm_squared += value * value;
        }

        if (!(norm_squared > 0.0L) ||
            !std::isfinite(static_cast<double>(norm_squared))) {
            throw std::domain_error(
                "ADF regression design matrix is rank deficient"
            );
        }

        const long double norm =
            std::sqrt(norm_squared);

        const long double x0 =
            static_cast<long double>(a(j, j));

        /*
            Choose the sign to avoid catastrophic cancellation.
        */
        const long double sign =
            x0 >= 0.0L ? 1.0L : -1.0L;

        const long double alpha =
            -sign * norm;

        std::vector<long double> v(n - j);

        for (std::size_t i = j; i < n; ++i) {
            v[i - j] =
                static_cast<long double>(a(i, j));
        }

        v[0] -= alpha;

        long double v_norm_squared = 0.0L;

        for (const long double value : v) {
            v_norm_squared += value * value;
        }

        if (!(v_norm_squared > 0.0L) ||
            !std::isfinite(
                static_cast<double>(v_norm_squared)
            )) {
            throw std::domain_error(
                "ADF Householder vector is degenerate"
            );
        }

        /*
            A := H A
            y := H y

            H = I - 2 vv'/(v'v)
        */
        for (std::size_t col = j; col < k; ++col) {

            long double projection = 0.0L;

            for (std::size_t i = j; i < n; ++i) {
                projection +=
                    v[i - j] *
                    static_cast<long double>(
                        a(i, col)
                    );
            }

            const long double factor =
                2.0L * projection / v_norm_squared;

            for (std::size_t i = j; i < n; ++i) {
                const long double updated =
                    static_cast<long double>(
                        a(i, col)
                    ) -
                    factor * v[i - j];

                a(i, col) =
                    static_cast<double>(updated);
            }
        }

        {
            long double projection = 0.0L;

            for (std::size_t i = j; i < n; ++i) {
                projection +=
                    v[i - j] *
                    static_cast<long double>(
                        transformed_y[i]
                    );
            }

            const long double factor =
                2.0L * projection / v_norm_squared;

            for (std::size_t i = j; i < n; ++i) {
                const long double updated =
                    static_cast<long double>(
                        transformed_y[i]
                    ) -
                    factor * v[i - j];

                transformed_y[i] =
                    static_cast<double>(updated);
            }
        }

        /*
            Explicitly establish the triangular structure.
            This also prevents tiny numerical values below the
            diagonal from contaminating later steps.
        */
        for (std::size_t i = j + 1; i < n; ++i) {
            a(i, j) = 0.0;
        }

        a(j, j) =
            static_cast<double>(alpha);

        if (!std::isfinite(a(j, j))) {
            throw std::domain_error(
                "ADF QR factorization produced non-finite value"
            );
        }
    }

    /*
        Back-substitution:

            R beta = Q'y
    */
    std::vector<double> coefficients(k, 0.0);

    for (std::size_t i = k; i-- > 0;) {

        long double value =
            static_cast<long double>(
                transformed_y[i]
            );

        for (std::size_t j = i + 1; j < k; ++j) {
            value -=
                static_cast<long double>(
                    a(i, j)
                ) *
                static_cast<long double>(
                    coefficients[j]
                );
        }

        const double diagonal = a(i, i);

        if (!std::isfinite(diagonal) ||
            std::abs(diagonal) <=
                std::numeric_limits<double>::epsilon()) {
            throw std::domain_error(
                "ADF regression matrix is rank deficient"
            );
        }

        coefficients[i] =
            static_cast<double>(
                value /
                static_cast<long double>(diagonal)
            );

        if (!std::isfinite(coefficients[i])) {
            throw std::domain_error(
                "ADF regression produced non-finite coefficient"
            );
        }
    }

    /*
        RSS = ||y - X beta||^2
    */
    long double rss = 0.0L;

    for (std::size_t i = 0; i < n; ++i) {

        long double fitted =
            static_cast<long double>(
                coefficients[0]
            ) *
            static_cast<long double>(
                x_gamma[i]
            );

        for (std::size_t j = 0; j < controls.size(); ++j) {
            fitted +=
                static_cast<long double>(
                    coefficients[j + 1]
                ) *
                static_cast<long double>(
                    controls[j][i]
                );
        }

        const long double residual =
            static_cast<long double>(y[i]) -
            fitted;

        rss += residual * residual;
    }

    if (!(rss > 0.0L) ||
        !std::isfinite(static_cast<double>(rss))) {
        throw std::domain_error(
            "ADF regression has invalid residual sum of squares"
        );
    }

    const long double sigma_squared =
        rss /
        static_cast<long double>(n - k);

    if (!(sigma_squared > 0.0L) ||
        !std::isfinite(
            static_cast<double>(sigma_squared)
        )) {
        throw std::domain_error(
            "ADF regression has invalid variance estimate"
        );
    }

    /*
        For X = Q R:

            X'X = R'R

        We need:

            (X'X)^(-1)_(0,0)

        Let

            R' z = e_0

        Then

            ||z||^2
              = e_0' R^(-1) R^(-T) e_0
              = (X'X)^(-1)_(0,0)
    */

    std::vector<long double> z(k, 0.0L);

    /*
        Solve R' z = e_0.

        R' is lower triangular.
    */
    for (std::size_t i = 0; i < k; ++i) {

        long double rhs =
            i == 0 ? 1.0L : 0.0L;

        for (std::size_t j = 0; j < i; ++j) {
            rhs -=
                static_cast<long double>(
                    a(j, i)
                ) *
                z[j];
        }

        const double diagonal = a(i, i);

        if (!std::isfinite(diagonal) ||
            std::abs(diagonal) <=
                std::numeric_limits<double>::epsilon()) {
            throw std::domain_error(
                "ADF covariance calculation is singular"
            );
        }

        z[i] =
            rhs /
            static_cast<long double>(diagonal);

        if (!std::isfinite(
                static_cast<double>(z[i]))) {
            throw std::domain_error(
                "ADF covariance calculation produced "
                "non-finite value"
            );
        }
    }

    long double inverse_xx_00 = 0.0L;

    for (const long double value : z) {
        inverse_xx_00 += value * value;
    }

    const long double variance_gamma =
        sigma_squared * inverse_xx_00;

    if (!(variance_gamma > 0.0L) ||
        !std::isfinite(
            static_cast<double>(variance_gamma)
        )) {
        throw std::domain_error(
            "ADF gamma variance is invalid"
        );
    }

    const double standard_error =
        std::sqrt(
            static_cast<double>(variance_gamma)
        );

    if (!std::isfinite(standard_error) ||
        !(standard_error > 0.0)) {
        throw std::domain_error(
            "ADF gamma standard error is invalid"
        );
    }

    return RegressionResult{
        coefficients[0],
        standard_error
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
            "ADF test requires more observations "
            "for requested lag count"
        );
    }

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

    const std::size_t first = lags + 1;

    const std::size_t observations =
        series.size() - first;

    y.reserve(observations);
    x_gamma.reserve(observations);

    for (auto& control : controls) {
        control.reserve(observations);
    }

    for (std::size_t i = 0; i < observations; ++i) {
        const std::size_t t = first + i;

        const double current =
            series[t].value;

        const double previous =
            series[t - 1].value;

        const double delta =
            current - previous;

        y.push_back(delta);
        x_gamma.push_back(previous);

        if (deterministic == DeterministicTerm::Intercept ||
            deterministic == DeterministicTerm::InterceptAndTrend) {
            controls[0].push_back(1.0);
        }

        if (deterministic == DeterministicTerm::InterceptAndTrend) {
            controls[1].push_back(
                static_cast<double>(t)
            );
        }
    }

    for (std::size_t lag = 1; lag <= lags; ++lag) {

        const std::size_t index =
            lag - 1;

        for (std::size_t i = 0; i < observations; ++i) {

            const std::size_t t =
                first + i;

            const double delta =
                series[t - lag].value -
                series[t - lag - 1].value;

            delta_lags[index].push_back(delta);
        }
    }

    for (auto& lag_column : delta_lags) {
        controls.push_back(
            std::move(lag_column)
        );
    }

    const RegressionResult result =
        regress_gamma(
            y,
            x_gamma,
            controls
        );

    const double statistic =
        result.coefficient /
        result.standard_error;

    if (!std::isfinite(statistic)) {
        throw std::domain_error(
            "ADF statistic is non-finite"
        );
    }

        ADFResult output{};

    output.statistic = statistic;
    output.gamma = result.coefficient;
    output.standard_error = result.standard_error;
    output.lags = lags;
    output.deterministic = deterministic;

    return output;
}

} // namespace quant::math