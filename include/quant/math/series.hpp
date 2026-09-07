#pragma once

#include "quant/math/observation.hpp"

#include <cstddef>
#include <vector>

namespace quant::math {

class Series {
public:
    Series() = default;

    void add(Observation observation);
    void reserve(std::size_t capacity);

    [[nodiscard]]
    std::size_t size() const noexcept;

    [[nodiscard]]
    bool empty() const noexcept;

    [[nodiscard]]
    const Observation& operator[](std::size_t index) const noexcept;

    [[nodiscard]]
    const Observation& at(std::size_t index) const;

    [[nodiscard]]
    auto begin() const noexcept {
        return data_.cbegin();
    }

    [[nodiscard]]
    auto end() const noexcept {
        return data_.cend();
    }

private:
    std::vector<Observation> data_;
};

} // namespace quant::math