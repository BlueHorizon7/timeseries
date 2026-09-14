#include "quant/math/adf_selection.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace quant::math {

namespace {

struct RegressionFit {
    double rss{};
    std::size_t observations{};
    std::size_t parameters{};
};

struct Candidate {
    std::size_t lag{};
    double criterion{};
};

RegressionFit fit_adf_candidate(
    const Series& series,
    std::size_t candidate_lags,
    std::size_t max_lags,
    DeterministicTerm deterministic
) {
    /*
        All candidate models are evaluated on the same effective
        sample determined by max_lags.

        Regression:

            ΔS_t =
                gamma * S_(t-1)
                + deterministic terms
                + delta_1 * ΔS_(t-1)
                + ...
                + delta_p * ΔS_(t-p)
                + error_t
    */

    if (candidate_lags > max_lags) {
        throw std::invalid_argument(
            "Candidate ADF lag exceeds maximum lag"
        );
    }

    if (series.size() <= max_lags + 1) {
        throw std::invalid_argument(
            "ADF lag selection requires more observations"
        );
    }

    const std::size_t first =
        max_lags + 1;

    const std::size_t n =
        series.size() - first;

    std::size_t k = 1;

    if (deterministic == DeterministicTerm::Intercept ||
        deterministic == DeterministicTerm::InterceptAndTrend) {
        ++k;
    }

    if (deterministic == DeterministicTerm::InterceptAndTrend) {
        ++k;
    }

    k += candidate_lags;

    if (n <= k) {
        throw std::invalid_argument(
            "ADF lag candidate has insufficient observations"
        );
    }

    std::vector<std::vector<double>> matrix(
        k,
        std::vector<double>(k + 1, 0.0)
    );

    for (std::size_t t = first;
         t < series.size();
         ++t) {

        std::vector<double> row;
        row.reserve(k);

        row.push_back(
            series[t - 1].value
        );

        if (deterministic == DeterministicTerm::Intercept ||
            deterministic == DeterministicTerm::InterceptAndTrend) {

            row.push_back(1.0);
        }

        if (deterministic == DeterministicTerm::InterceptAndTrend) {
            row.push_back(
                static_cast<double>(t)
            );
        }

        for (std::size_t lag = 1;
             lag <= candidate_lags;
             ++lag) {

            row.push_back(
                series[t - lag].value -
                series[t - lag - 1].value
            );
        }

        const double dependent =
            series[t].value -
            series[t - 1].value;

        for (std::size_t r = 0;
             r < k;
             ++r) {

            for (std::size_t c = 0;
                 c < k;
                 ++c) {

                matrix[r][c] +=
                    row[r] * row[c];
            }

            matrix[r][k] +=
                row[r] * dependent;
        }
    }

    /*
        Gaussian elimination with partial pivoting.
    */
    for (std::size_t col = 0;
         col < k;
         ++col) {

        std::size_t pivot = col;

        for (std::size_t row = col + 1;
             row < k;
             ++row) {

            if (std::abs(matrix[row][col]) >
                std::abs(matrix[pivot][col])) {

                pivot = row;
            }
        }

        if (std::abs(matrix[pivot][col]) < 1e-14) {
            throw std::domain_error(
                "ADF lag-selection regression is singular"
            );
        }

        if (pivot != col) {
            std::swap(
                matrix[col],
                matrix[pivot]
            );
        }

        for (std::size_t row = col + 1;
             row < k;
             ++row) {

            const double factor =
                matrix[row][col] /
                matrix[col][col];

            for (std::size_t c = col;
                 c <= k;
                 ++c) {

                matrix[row][c] -=
                    factor * matrix[col][c];
            }
        }
    }

    /*
        Back substitution.
    */
    std::vector<double> coefficients(k);

    for (std::size_t i = k;
         i-- > 0;) {

        double value =
            matrix[i][k];

        for (std::size_t j = i + 1;
             j < k;
             ++j) {

            value -=
                matrix[i][j] *
                coefficients[j];
        }

        coefficients[i] =
            value / matrix[i][i];
    }

    /*
        Residual sum of squares.
    */
    double rss = 0.0;

    for (std::size_t t = first;
         t < series.size();
         ++t) {

        std::vector<double> row;
        row.reserve(k);

        row.push_back(
            series[t - 1].value
        );

        if (deterministic == DeterministicTerm::Intercept ||
            deterministic == DeterministicTerm::InterceptAndTrend) {

            row.push_back(1.0);
        }

        if (deterministic == DeterministicTerm::InterceptAndTrend) {
            row.push_back(
                static_cast<double>(t)
            );
        }

        for (std::size_t lag = 1;
             lag <= candidate_lags;
             ++lag) {

            row.push_back(
                series[t - lag].value -
                series[t - lag - 1].value
            );
        }

        double fitted = 0.0;

        for (std::size_t j = 0;
             j < k;
             ++j) {

            fitted +=
                coefficients[j] *
                row[j];
        }

        const double dependent =
            series[t].value -
            series[t - 1].value;

        const double residual =
            dependent - fitted;

        rss +=
            residual * residual;
    }

    if (!std::isfinite(rss)) {
        throw std::domain_error(
            "ADF lag-selection regression produced "
            "a non-finite residual sum of squares"
        );
    }

    return RegressionFit{
        rss,
        n,
        k
    };
}

double information_criterion(
    const RegressionFit& fit,
    InformationCriterion criterion
) {
    if (fit.observations == 0) {
        throw std::domain_error(
            "Cannot calculate information criterion "
            "with zero observations"
        );
    }

    if (fit.rss <= 0.0 ||
        !std::isfinite(fit.rss)) {

        throw std::domain_error(
            "ADF candidate has invalid residual "
            "sum of squares"
        );
    }

    const double n =
        static_cast<double>(
            fit.observations
        );

    const double k =
        static_cast<double>(
            fit.parameters
        );

    const double log_likelihood_term =
        n *
        std::log(
            fit.rss / n
        );

    switch (criterion) {
        case InformationCriterion::AIC:
            return log_likelihood_term +
                   2.0 * k;

        case InformationCriterion::BIC:
            return log_likelihood_term +
                   k * std::log(n);
    }

    throw std::invalid_argument(
        "Unknown information criterion"
    );
}

} // namespace

ADFLagSelectionResult select_adf_lag(
    const Series& series,
    std::size_t max_lags,
    DeterministicTerm deterministic,
    InformationCriterion criterion
) {
    if (series.size() < 20) {
        throw std::invalid_argument(
            "ADF lag selection requires at least "
            "20 observations"
        );
    }

    if (max_lags >= series.size() - 2) {
        throw std::invalid_argument(
            "Maximum ADF lag is too large for the series"
        );
    }

    std::vector<Candidate> candidates;

    candidates.reserve(max_lags + 1);

    /*
        Evaluate all candidate models.
    */
    for (std::size_t lag = 0;
         lag <= max_lags;
         ++lag) {

        try {
            const auto fit =
                fit_adf_candidate(
                    series,
                    lag,
                    max_lags,
                    deterministic
                );

            const double value =
                information_criterion(
                    fit,
                    criterion
                );

            if (!std::isfinite(value)) {
                continue;
            }

            candidates.push_back(
                Candidate{
                    lag,
                    value
                }
            );
        }
        catch (const std::domain_error&) {
            /*
                Singular/invalid candidate.
            */
            continue;
        }
        catch (const std::invalid_argument&) {
            /*
                Candidate cannot be estimated.
            */
            continue;
        }
    }

    if (candidates.empty()) {
        throw std::domain_error(
            "No valid ADF lag candidate"
        );
    }

    /*
        Best information criterion first.

        Ties are resolved in favor of the smaller lag.
    */
    std::sort(
        candidates.begin(),
        candidates.end(),
        [](const Candidate& lhs,
           const Candidate& rhs) {

            if (lhs.criterion != rhs.criterion) {
                return lhs.criterion <
                       rhs.criterion;
            }

            return lhs.lag < rhs.lag;
        }
    );

    /*
        The existing public ADF implementation uses a
        lag-specific effective sample. Therefore a candidate
        that is valid for the common-sample IC regression may
        still be singular in the actual ADF implementation.

        Try candidates in IC order and select the first one
        that the authoritative ADF implementation can evaluate.
    */
    for (const auto& candidate : candidates) {
        try {
            const auto adf =
                augmented_dickey_fuller(
                    series,
                    candidate.lag,
                    deterministic
                );

            return ADFLagSelectionResult{
                candidate.lag,
                candidate.criterion,
                adf
            };
        }
        catch (const std::domain_error&) {
            /*
                Candidate cannot be evaluated by the
                authoritative ADF implementation.
                Try the next-best candidate.
            */
            continue;
        }
        catch (const std::invalid_argument&) {
            continue;
        }
    }

    throw std::domain_error(
        "No ADF lag candidate could be evaluated "
        "by the ADF implementation"
    );
}

} // namespace quant::math