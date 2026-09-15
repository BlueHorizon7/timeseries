#include "quant/research/walk_forward_parameter_stability.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace quant::research {

namespace {

void validate_parameter_value(
    SensitivityParameter parameter,
    double value
) {
    if (!std::isfinite(value)) {
        throw std::invalid_argument(
            "Walk-forward parameter values must be finite"
        );
    }

    switch (parameter) {
    case SensitivityParameter::HedgeRatioWindow:
    case SensitivityParameter::ZScoreWindow:
        if (value < 2.0 ||
            std::floor(value) != value) {
            throw std::invalid_argument(
                "Window parameter values must be integers >= 2"
            );
        }
        break;

    case SensitivityParameter::EntryZScore:
        if (value <= 0.0) {
            throw std::invalid_argument(
                "Entry z-score must be positive"
            );
        }
        break;

    case SensitivityParameter::ExitZScore:
        if (value < 0.0) {
            throw std::invalid_argument(
                "Exit z-score must be non-negative"
            );
        }
        break;
    }
}

void set_parameter(
    WalkForwardParameters& parameters,
    SensitivityParameter parameter,
    double value
) {
    switch (parameter) {
    case SensitivityParameter::HedgeRatioWindow:
        parameters.strategy.hedge_ratio_window =
            static_cast<std::size_t>(value);
        break;

    case SensitivityParameter::ZScoreWindow:
        parameters.strategy.zscore_window =
            static_cast<std::size_t>(value);
        break;

    case SensitivityParameter::EntryZScore:
    parameters.strategy.signal_parameters.entry_zscore =
        value;
    break;

case SensitivityParameter::ExitZScore:
    parameters.strategy.signal_parameters.exit_zscore =
        value;
    break;
    }
}

double mean(
    const std::vector<double>& values
) {
    if (values.empty()) {
        return 0.0;
    }

    const double sum =
        std::accumulate(
            values.begin(),
            values.end(),
            0.0
        );

    return sum /
        static_cast<double>(values.size());
}

double standard_deviation(
    const std::vector<double>& values
) {
    if (values.size() < 2) {
        return 0.0;
    }

    const double average = mean(values);

    double squared_sum = 0.0;

    for (const double value : values) {
        const double deviation =
            value - average;

        squared_sum +=
            deviation * deviation;
    }

    return std::sqrt(
        squared_sum /
        static_cast<double>(values.size() - 1)
    );
}

std::vector<double> rank_values(
    const std::vector<double>& values
) {
    const std::size_t n = values.size();

    std::vector<std::pair<double, std::size_t>> ordered;
    ordered.reserve(n);

    for (std::size_t i = 0; i < n; ++i) {
        ordered.emplace_back(values[i], i);
    }

    std::stable_sort(
        ordered.begin(),
        ordered.end(),
        [](
            const auto& lhs,
            const auto& rhs
        ) {
            return lhs.first > rhs.first;
        }
    );

    std::vector<double> ranks(n);

    /*
     * Competition ranking:
     *
     * 10, 8, 8, 4
     *
     * becomes
     *
     * 1, 2, 2, 4
     */
    std::size_t position = 0;

    while (position < n) {
        std::size_t next = position + 1;

        while (next < n &&
               ordered[next].first ==
                   ordered[position].first) {
            ++next;
        }

        const double rank =
            1.0 +
            static_cast<double>(position);

        for (std::size_t i = position;
             i < next;
             ++i) {
            ranks[ordered[i].second] = rank;
        }

        position = next;
    }

    return ranks;
}

} // namespace

WalkForwardParameterStabilityResult
analyze_walk_forward_parameter_stability(
    const quant::data::AlignedSeries& data,
    const WalkForwardParameters& baseline_parameters,
    double periods_per_year,
    SensitivityParameter parameter,
    const std::vector<double>& values
) {
    if (values.empty()) {
        throw std::invalid_argument(
            "Walk-forward parameter stability requires "
            "at least one parameter value"
        );
    }

    if (!std::isfinite(periods_per_year) ||
        periods_per_year <= 0.0) {
        throw std::invalid_argument(
            "Periods per year must be finite and positive"
        );
    }

    for (const double value : values) {
        validate_parameter_value(parameter, value);
    }

    WalkForwardParameterStabilityResult result;
    result.parameter = parameter;

    std::vector<
        std::vector<double>
    > fold_total_returns(values.size());

    std::vector<
        std::vector<double>
    > fold_sharpes(values.size());

    std::size_t fold_count = 0;

    for (std::size_t value_index = 0;
         value_index < values.size();
         ++value_index) {

        WalkForwardParameters parameters =
            baseline_parameters;

        set_parameter(
            parameters,
            parameter,
            values[value_index]
        );

        const WalkForwardResult walk_forward =
            run_walk_forward(
                data,
                parameters,
                periods_per_year
            );

        if (value_index == 0) {
            fold_count =
                walk_forward.folds.size();
        } else if (
            walk_forward.folds.size() != fold_count
        ) {
            throw std::runtime_error(
                "Walk-forward runs produced inconsistent "
                "fold counts"
            );
        }

        fold_total_returns[value_index].reserve(
            fold_count
        );

        fold_sharpes[value_index].reserve(
            fold_count
        );

        for (const auto& fold :
             walk_forward.folds) {

            if (!fold.performance.has_value()) {
                continue;
            }

            const auto& performance =
                fold.performance.value();

            if (!std::isfinite(
                    performance.total_return
                ) ||
                !std::isfinite(
                    performance.sharpe_ratio
                )) {
                throw std::domain_error(
                    "Walk-forward fold performance "
                    "contains non-finite values"
                );
            }

            fold_total_returns[value_index]
                .push_back(
                    performance.total_return
                );

            fold_sharpes[value_index]
                .push_back(
                    performance.sharpe_ratio
                );
        }
    }

    result.number_of_folds = fold_count;

    /*
     * Construct fold rankings.
     *
     * Each fold only ranks parameter values for which
     * that parameter's walk-forward run produced a
     * performance observation.
     */
    std::vector<std::vector<double>>
        total_return_by_fold(
            fold_count
        );

    std::vector<std::vector<double>>
        sharpe_by_fold(
            fold_count
        );

    /*
     * Re-run the walk-forward results so that rank
     * construction is explicitly fold-aligned.
     */
    std::vector<
        WalkForwardResult
    > walk_forward_results;

    walk_forward_results.reserve(values.size());

    for (const double value : values) {
        WalkForwardParameters parameters =
            baseline_parameters;

        set_parameter(
            parameters,
            parameter,
            value
        );

        walk_forward_results.push_back(
            run_walk_forward(
                data,
                parameters,
                periods_per_year
            )
        );
    }

    std::vector<double> total_rank_sum(values.size(), 0.0);
    std::vector<double> total_rank_squared_sum(values.size(), 0.0);
    std::vector<std::size_t> total_rank_count(values.size(), 0);

    std::vector<double> sharpe_rank_sum(values.size(), 0.0);
    std::vector<double> sharpe_rank_squared_sum(values.size(), 0.0);
    std::vector<std::size_t> sharpe_rank_count(values.size(), 0);

    for (std::size_t fold_index = 0;
         fold_index < fold_count;
         ++fold_index) {

        std::vector<double> total_values;
        std::vector<std::size_t> total_indices;

        std::vector<double> sharpe_values;
        std::vector<std::size_t> sharpe_indices;

        for (std::size_t value_index = 0;
             value_index < values.size();
             ++value_index) {

            const auto& folds =
                walk_forward_results[value_index].folds;

            if (!folds[fold_index].performance.has_value()) {
                continue;
            }

            const auto& performance =
                folds[fold_index].performance.value();

            total_values.push_back(
                performance.total_return
            );

            total_indices.push_back(value_index);

            sharpe_values.push_back(
                performance.sharpe_ratio
            );

            sharpe_indices.push_back(value_index);
        }

        const auto total_ranks =
            rank_values(total_values);

        for (std::size_t i = 0;
             i < total_indices.size();
             ++i) {

            const std::size_t value_index =
                total_indices[i];

            const double rank =
                total_ranks[i];

            total_rank_sum[value_index] += rank;

            total_rank_squared_sum[value_index] +=
                rank * rank;

            ++total_rank_count[value_index];
        }

        const auto sharpe_ranks =
            rank_values(sharpe_values);

        for (std::size_t i = 0;
             i < sharpe_indices.size();
             ++i) {

            const std::size_t value_index =
                sharpe_indices[i];

            const double rank =
                sharpe_ranks[i];

            sharpe_rank_sum[value_index] += rank;

            sharpe_rank_squared_sum[value_index] +=
                rank * rank;

            ++sharpe_rank_count[value_index];
        }
    }

    result.points.reserve(values.size());

    for (std::size_t value_index = 0;
         value_index < values.size();
         ++value_index) {

        const auto& returns =
            fold_total_returns[value_index];

        const auto& sharpes =
            fold_sharpes[value_index];

        WalkForwardParameterStabilityPoint point;

        point.parameter = parameter;
        point.parameter_value = values[value_index];

        point.fold_total_returns = returns;
        point.fold_sharpes = sharpes;

        point.mean_total_return =
            mean(returns);

        point.standard_deviation_total_return =
            standard_deviation(returns);

        point.mean_sharpe =
            mean(sharpes);

        point.standard_deviation_sharpe =
            standard_deviation(sharpes);

        point.number_of_observed_folds =
            returns.size();

        if (total_rank_count[value_index] > 0) {
            point.mean_total_return_rank =
                total_rank_sum[value_index] /
                static_cast<double>(
                    total_rank_count[value_index]
                );

            if (total_rank_count[value_index] > 1) {
                const double mean_rank =
                    point.mean_total_return_rank;

                const double second_moment =
                    total_rank_squared_sum[value_index] /
                    static_cast<double>(
                        total_rank_count[value_index]
                    );

                const double variance =
                    std::max(
                        0.0,
                        second_moment -
                        mean_rank * mean_rank
                    );

                point.standard_deviation_total_return_rank =
                    std::sqrt(variance);
            }
        }

        if (sharpe_rank_count[value_index] > 0) {
            point.mean_sharpe_rank =
                sharpe_rank_sum[value_index] /
                static_cast<double>(
                    sharpe_rank_count[value_index]
                );

            if (sharpe_rank_count[value_index] > 1) {
                const double mean_rank =
                    point.mean_sharpe_rank;

                const double second_moment =
                    sharpe_rank_squared_sum[value_index] /
                    static_cast<double>(
                        sharpe_rank_count[value_index]
                    );

                const double variance =
                    std::max(
                        0.0,
                        second_moment -
                        mean_rank * mean_rank
                    );

                point.standard_deviation_sharpe_rank =
                    std::sqrt(variance);
            }
        }

        result.points.push_back(
            std::move(point)
        );
    }

    return result;
}

} // namespace quant::research