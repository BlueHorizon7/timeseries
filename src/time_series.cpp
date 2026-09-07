#include "quant/data/time_series.hpp"

#include <stdexcept>

namespace quant::data {

void TimeSeries::add(Candle candle) {
    if (!data_.empty() &&
        candle.timestamp <= data_.back().timestamp) {

        throw std::invalid_argument(
            "Candle timestamps must be strictly increasing"
        );
    }

    data_.push_back(candle);
}

void TimeSeries::reserve(std::size_t capacity) {
    data_.reserve(capacity);
}

std::size_t TimeSeries::size() const noexcept {
    return data_.size();
}

bool TimeSeries::empty() const noexcept {
    return data_.empty();
}

const Candle& TimeSeries::operator[](std::size_t index) const noexcept {
    return data_[index];
}

const Candle& TimeSeries::at(std::size_t index) const {
    return data_.at(index);
}

} // namespace quant::data