#include "quant/research/validation.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::research {

DataSplit train_test_split(
    const quant::data::AlignedSeries& data,
    double training_fraction
) {
    if (data.size() < 2) {
        throw std::invalid_argument(
            "Train/test split requires at least two observations"
        );
    }

    if (!std::isfinite(training_fraction) ||
        training_fraction <= 0.0 ||
        training_fraction >= 1.0) {
        throw std::invalid_argument(
            "Training fraction must be between 0 and 1"
        );
    }

    const std::size_t training_size =
        static_cast<std::size_t>(
            std::floor(
                static_cast<double>(data.size()) *
                training_fraction
            )
        );

    if (training_size == 0 ||
        training_size >= data.size()) {
        throw std::invalid_argument(
            "Training split leaves no testing observations"
        );
    }

    DataSplit result;

    result.training.reserve(training_size);
    result.testing.reserve(data.size() - training_size);

    for (std::size_t i = 0; i < training_size; ++i) {
        result.training.add(data[i]);
    }

    for (std::size_t i = training_size;
         i < data.size();
         ++i) {
        result.testing.add(data[i]);
    }

    return result;
}

std::vector<WalkForwardWindow>
make_walk_forward_windows(
    std::size_t number_of_observations,
    std::size_t training_size,
    std::size_t testing_size,
    std::size_t step
) {
    if (training_size == 0 ||
        testing_size == 0 ||
        step == 0) {
        throw std::invalid_argument(
            "Walk-forward sizes and step must be positive"
        );
    }

    if (number_of_observations <
        training_size + testing_size) {
        throw std::invalid_argument(
            "Not enough observations for walk-forward validation"
        );
    }

    std::vector<WalkForwardWindow> result;

    for (std::size_t test_begin = training_size;
         test_begin + testing_size <= number_of_observations;
         test_begin += step) {

        result.push_back(
            WalkForwardWindow{
                0,
                test_begin,
                test_begin,
                test_begin + testing_size
            }
        );
    }

    return result;
}

} // namespace quant::research