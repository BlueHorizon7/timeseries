#pragma once

#include "quant/data/candle.hpp"

#include <cstddef>
#include <vector>

namespace quant::data {

class TimeSeries {
public:
    TimeSeries() = default;

    void add(Candle candle);
    void reserve(std::size_t capacity);

    [[nodiscard]]
    std::size_t size() const noexcept;

    [[nodiscard]]
    bool empty() const noexcept;

    [[nodiscard]]
    const Candle& operator[](std::size_t index) const noexcept;

    [[nodiscard]]
    const Candle& at(std::size_t index) const;

    [[nodiscard]]
    auto begin() const noexcept {
        return data_.cbegin();
    }

    [[nodiscard]]
    auto end() const noexcept {
        return data_.cend();
    }

    [[nodiscard]]
    auto cbegin() const noexcept {
        return data_.cbegin();
    }

    [[nodiscard]]
    auto cend() const noexcept {
        return data_.cend();
    }

private:
    std::vector<Candle> data_;
};

} // namespace quant::data