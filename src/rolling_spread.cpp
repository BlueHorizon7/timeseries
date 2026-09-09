#include "quant/math/rolling_spread.hpp"

#include <stdexcept>

namespace quant::math {

Series rolling_spread(
    const Series& x,
    const Series& y,
    std::size_t window
) {
    if (x.size() != y.size()) {
        throw std::invalid_argument(
            "Rolling spread requires equal-length series"
        );
    }

    if (window < 2) {
        throw std::invalid_argument(
            "Rolling spread requires a window of at least two observations"
        );
    }

    if (x.size() <= window) {
        throw std::invalid_argument(
            "Rolling spread requires observations beyond the estimation window"
        );
    }

    for (std::size_t i = 0; i < x.size(); ++i) {
        if (x[i].timestamp != y[i].timestamp) {
            throw std::invalid_argument(
                "Rolling spread requires aligned timestamps"
            );
        }
    }

    const auto models =
        rolling_ols(x, y, window);

    Series result;
    result.reserve(models.size());

    for (const auto& entry : models) {
        const std::size_t t =
            entry.timestamp_index;

        const double predicted =
            entry.model.intercept +
            entry.model.slope * x[t].value;

        const double spread =
            y[t].value - predicted;

        result.add(
            Observation{
                y[t].timestamp,
                spread
            }
        );
    }

    return result;
}

} // namespace quant::math