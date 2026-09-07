#include "quant/math/returns.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::math {

Series log_returns(const quant::data::TimeSeries& prices) {
    Series result;

    if (prices.size() < 2) {
        return result;
    }

    result.reserve(prices.size() - 1);

    for (std::size_t i = 1; i < prices.size(); ++i) {
        const double previous = prices[i - 1].close;
        const double current = prices[i].close;

        if (previous <= 0.0 || current <= 0.0) {
            throw std::domain_error(
                "Log return requires strictly positive prices"
            );
        }

        const double value =
            std::log(current / previous);

        result.add(Observation{
            prices[i].timestamp,
            value
        });
    }

    return result;
}

} // namespace quant::math