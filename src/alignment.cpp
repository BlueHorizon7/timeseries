#include "quant/data/alignment.hpp"

namespace quant::data {

AlignedSeries inner_join(
    const TimeSeries& x,
    const TimeSeries& y
) {
    AlignedSeries result;

    result.reserve(
        x.size() < y.size()
            ? x.size()
            : y.size()
    );

    std::size_t i = 0;
    std::size_t j = 0;

    while (i < x.size() && j < y.size()) {
        const auto x_timestamp =
            x[i].timestamp;

        const auto y_timestamp =
            y[j].timestamp;

        if (x_timestamp < y_timestamp) {
            ++i;
            continue;
        }

        if (y_timestamp < x_timestamp) {
            ++j;
            continue;
        }

        result.add(
            AlignedObservation{
                x_timestamp,
                x[i].close,
                y[j].close
            }
        );

        ++i;
        ++j;
    }

    return result;
}

} // namespace quant::data