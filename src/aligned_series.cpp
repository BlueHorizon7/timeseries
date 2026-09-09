#include "quant/data/aligned_series.hpp"

#include <stdexcept>

namespace quant::data {

void AlignedSeries::add(
    AlignedObservation observation
) {
    if (!data_.empty() &&
        observation.timestamp <= data_.back().timestamp) {

        throw std::invalid_argument(
            "Aligned timestamps must be strictly increasing"
        );
    }

    data_.push_back(observation);
}

void AlignedSeries::reserve(
    std::size_t capacity
) {
    data_.reserve(capacity);
}

std::size_t AlignedSeries::size() const noexcept {
    return data_.size();
}

bool AlignedSeries::empty() const noexcept {
    return data_.empty();
}

const AlignedObservation&
AlignedSeries::operator[](
    std::size_t index
) const noexcept {
    return data_[index];
}

const AlignedObservation&
AlignedSeries::at(
    std::size_t index
) const {
    return data_.at(index);
}

} // namespace quant::data