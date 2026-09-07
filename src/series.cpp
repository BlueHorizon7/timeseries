#include "quant/math/series.hpp"

#include <stdexcept>

namespace quant::math {

void Series::add(Observation observation) {
    if (!data_.empty() &&
        observation.timestamp <= data_.back().timestamp) {

        throw std::invalid_argument(
            "Observation timestamps must be strictly increasing"
        );
    }

    data_.push_back(observation);
}

void Series::reserve(std::size_t capacity) {
    data_.reserve(capacity);
}

std::size_t Series::size() const noexcept {
    return data_.size();
}

bool Series::empty() const noexcept {
    return data_.empty();
}

const Observation&
Series::operator[](std::size_t index) const noexcept {
    return data_[index];
}

const Observation&
Series::at(std::size_t index) const {
    return data_.at(index);
}

} // namespace quant::math