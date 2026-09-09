#pragma once

#include "quant/data/aligned_series.hpp"

#include <cstddef>
#include <vector>

namespace quant::research {

struct DataSplit {
    quant::data::AlignedSeries training;
    quant::data::AlignedSeries testing;
};

[[nodiscard]]
DataSplit
train_test_split(
    const quant::data::AlignedSeries& data,
    double training_fraction
);

struct WalkForwardWindow {
    std::size_t training_begin{};
    std::size_t training_end{};
    std::size_t testing_begin{};
    std::size_t testing_end{};
};

[[nodiscard]]
std::vector<WalkForwardWindow>
make_walk_forward_windows(
    std::size_t number_of_observations,
    std::size_t training_size,
    std::size_t testing_size,
    std::size_t step
);

} // namespace quant::research