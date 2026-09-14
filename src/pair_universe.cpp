#include "quant/research/pair_universe.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

namespace quant::research {

namespace {

struct ReturnObservation {
    std::int64_t timestamp{};
    double value{};
};

using ReturnSeries = std::vector<ReturnObservation>;

ReturnSeries make_log_returns(
    const quant::math::Series& prices
) {
    if (prices.size() < 2) {
        throw std::invalid_argument(
            "Price series requires at least two observations"
        );
    }

    ReturnSeries returns;
    returns.reserve(prices.size() - 1);

    for (std::size_t i = 1; i < prices.size(); ++i) {

        const double previous =
            prices[i - 1].value;

        const double current =
            prices[i].value;

        if (!std::isfinite(previous) ||
            !std::isfinite(current) ||
            previous <= 0.0 ||
            current <= 0.0) {

            throw std::invalid_argument(
                "Prices must be finite and strictly positive"
            );
        }

        const double value =
            std::log(current / previous);

        if (!std::isfinite(value)) {
            throw std::domain_error(
                "Log return is non-finite"
            );
        }

        returns.push_back(
            ReturnObservation{
                prices[i].timestamp,
                value
            }
        );
    }

    return returns;
}

struct AlignedReturns {
    std::vector<double> x;
    std::vector<double> y;
};

AlignedReturns align_returns(
    const ReturnSeries& x,
    const ReturnSeries& y
) {
    AlignedReturns result;

    result.x.reserve(
        std::min(x.size(), y.size())
    );

    result.y.reserve(
        std::min(x.size(), y.size())
    );

    std::size_t i = 0;
    std::size_t j = 0;

    while (i < x.size() && j < y.size()) {

        if (x[i].timestamp < y[j].timestamp) {
            ++i;
            continue;
        }

        if (y[j].timestamp < x[i].timestamp) {
            ++j;
            continue;
        }

        result.x.push_back(x[i].value);
        result.y.push_back(y[j].value);

        ++i;
        ++j;
    }

    return result;
}

double return_correlation(
    const std::vector<double>& x,
    const std::vector<double>& y
) {
    if (x.size() != y.size()) {
        throw std::logic_error(
            "Return vectors must have equal lengths"
        );
    }

    const std::size_t n = x.size();

    if (n < 2) {
        throw std::invalid_argument(
            "Correlation requires at least two observations"
        );
    }

    /*
        Two-pass centered covariance calculation.

        Using long double for the accumulators provides
        additional numerical headroom without changing the
        public representation.
    */
    long double sum_x = 0.0L;
    long double sum_y = 0.0L;

    for (std::size_t i = 0; i < n; ++i) {
        sum_x += static_cast<long double>(x[i]);
        sum_y += static_cast<long double>(y[i]);
    }

    const long double mean_x =
        sum_x / static_cast<long double>(n);

    const long double mean_y =
        sum_y / static_cast<long double>(n);

    long double sxx = 0.0L;
    long double syy = 0.0L;
    long double sxy = 0.0L;

    for (std::size_t i = 0; i < n; ++i) {

        const long double dx =
            static_cast<long double>(x[i]) -
            mean_x;

        const long double dy =
            static_cast<long double>(y[i]) -
            mean_y;

        sxx += dx * dx;
        syy += dy * dy;
        sxy += dx * dy;
    }

    if (!(sxx > 0.0L) ||
        !(syy > 0.0L)) {

        throw std::domain_error(
            "Return correlation undefined for "
            "zero-variance series"
        );
    }

    const long double denominator =
        std::sqrt(sxx * syy);

    if (!(denominator > 0.0L) ||
        !std::isfinite(
            static_cast<double>(denominator)
        )) {
        throw std::domain_error(
            "Return correlation denominator is invalid"
        );
    }

    const double correlation =
        static_cast<double>(sxy / denominator);

    if (!std::isfinite(correlation)) {
        throw std::domain_error(
            "Return correlation is non-finite"
        );
    }

    /*
        Guard against tiny floating-point excursions
        outside [-1, 1].
    */
    return std::clamp(
        correlation,
        -1.0,
        1.0
    );
}

} // namespace

std::vector<PairCandidate> generate_pair_candidates(
    const std::vector<AssetSeries>& universe,
    const PairUniverseParameters& parameters
) {
    if (universe.size() < 2) {
        throw std::invalid_argument(
            "Pair universe requires at least two assets"
        );
    }

    if (!std::isfinite(
            parameters.minimum_absolute_correlation
        ) ||
        parameters.minimum_absolute_correlation < 0.0 ||
        parameters.minimum_absolute_correlation > 1.0) {

        throw std::invalid_argument(
            "Minimum absolute correlation must be "
            "between zero and one"
        );
    }

    if (parameters.minimum_observations < 2) {
        throw std::invalid_argument(
            "Minimum observations must be at least two"
        );
    }

    /*
        Validate symbols before doing O(N^2) work.
    */
    for (std::size_t i = 0; i < universe.size(); ++i) {

        if (universe[i].symbol.empty()) {
            throw std::invalid_argument(
                "Asset symbol must not be empty"
            );
        }

        for (std::size_t j = 0; j < i; ++j) {
            if (universe[i].symbol ==
                universe[j].symbol) {

                throw std::invalid_argument(
                    "Asset symbols must be unique"
                );
            }
        }
    }

    /*
        Convert every price series to returns once.

        This avoids repeatedly recomputing returns for
        every pair.
    */
    std::vector<ReturnSeries> returns;

    returns.reserve(universe.size());

    for (const auto& asset : universe) {
        returns.push_back(
            make_log_returns(asset.prices)
        );
    }

    std::vector<PairCandidate> candidates;

    /*
        Maximum possible number of unordered pairs.
    */
    candidates.reserve(
        universe.size() *
        (universe.size() - 1) /
        2
    );

    for (std::size_t i = 0;
         i < universe.size();
         ++i) {

        for (std::size_t j = i + 1;
             j < universe.size();
             ++j) {

            const auto aligned =
                align_returns(
                    returns[i],
                    returns[j]
                );

            if (aligned.x.size() <
                parameters.minimum_observations) {
                continue;
            }

            const double correlation =
                return_correlation(
                    aligned.x,
                    aligned.y
                );

            if (std::abs(correlation) <
                parameters.minimum_absolute_correlation) {
                continue;
            }

            candidates.push_back(
                PairCandidate{
                    i,
                    j,
                    correlation,
                    aligned.x.size()
                }
            );
        }
    }

    /*
        Deterministic ranking:
          1. descending |correlation|
          2. lower x index
          3. lower y index
    */
    std::sort(
        candidates.begin(),
        candidates.end(),
        [](const PairCandidate& lhs,
           const PairCandidate& rhs) {

            const double lhs_strength =
                std::abs(lhs.correlation);

            const double rhs_strength =
                std::abs(rhs.correlation);

            if (lhs_strength != rhs_strength) {
                return lhs_strength >
                       rhs_strength;
            }

            if (lhs.x_index != rhs.x_index) {
                return lhs.x_index <
                       rhs.x_index;
            }

            return lhs.y_index <
                   rhs.y_index;
        }
    );

    if (parameters.maximum_candidates != 0 &&
        candidates.size() >
            parameters.maximum_candidates) {

        candidates.resize(
            parameters.maximum_candidates
        );
    }

    return candidates;
}

} // namespace quant::research