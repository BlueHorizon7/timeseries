#pragma once

#include "quant/data/aligned_series.hpp"
#include "quant/math/cointegration.hpp"

#include <cstddef>

namespace quant::research {

struct FormationParameters {
    std::size_t formation_size{};

    quant::math::CointegrationParameters
        cointegration{};
};

struct FormationResult {
    quant::data::AlignedSeries formation_data;

    quant::data::AlignedSeries trading_data;

    quant::math::CointegrationResult
        cointegration;

    bool passes{};
};

[[nodiscard]]
FormationResult run_formation_test(
    const quant::data::AlignedSeries& data,
    std::size_t formation_size
);

[[nodiscard]]
FormationResult run_formation_test(
    const quant::data::AlignedSeries& data,
    const FormationParameters&
        parameters
);

} // namespace quant::research