#pragma once

#include "quant/math/observation.hpp"

#include <cstddef>
#include <vector>

namespace quant::data {

struct AlignedObservation {
    std::int64_t timestamp{};
    double x{};
    double y{};
};

class AlignedSeries {
public:
    AlignedSeries() = default;

    void add(AlignedObservation observation);

    void reserve(std::size_t capacity);

    [[nodiscard]]
    std::size_t size() const noexcept;

    [[nodiscard]]
    bool empty() const noexcept;

    [[nodiscard]]
    const AlignedObservation&
    operator[](std::size_t index) const noexcept;

    [[nodiscard]]
    const AlignedObservation&
    at(std::size_t index) const;

    [[nodiscard]]
    auto begin() const noexcept {
        return data_.cbegin();
    }

    [[nodiscard]]
    auto end() const noexcept {
        return data_.cend();
    }

private:
    std::vector<AlignedObservation> data_;
};

} // namespace quant::data