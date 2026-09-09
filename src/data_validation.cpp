#include "quant/data/validation.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::data {

void validate_market_data(
    const TimeSeries& series
) {
    if (series.empty()) {
        throw std::invalid_argument(
            "Market data must not be empty"
        );
    }

    for (std::size_t i = 0;
         i < series.size();
         ++i) {

        const auto& candle = series[i];

        if (candle.timestamp <= 0) {
            throw std::domain_error(
                "Timestamp must be positive"
            );
        }

        if (!std::isfinite(candle.open) ||
            !std::isfinite(candle.high) ||
            !std::isfinite(candle.low) ||
            !std::isfinite(candle.close) ||
            !std::isfinite(candle.volume)) {

            throw std::domain_error(
                "Market data contains non-finite values"
            );
        }

        if (candle.open <= 0.0 ||
            candle.high <= 0.0 ||
            candle.low <= 0.0 ||
            candle.close <= 0.0) {

            throw std::domain_error(
                "Market prices must be positive"
            );
        }

        if (candle.high < candle.low) {
            throw std::domain_error(
                "High price cannot be below low price"
            );
        }

        if (candle.open < candle.low ||
            candle.open > candle.high) {

            throw std::domain_error(
                "Open price lies outside candle range"
            );
        }

        if (candle.close < candle.low ||
            candle.close > candle.high) {

            throw std::domain_error(
                "Close price lies outside candle range"
            );
        }

        if (candle.volume < 0.0) {
            throw std::domain_error(
                "Volume cannot be negative"
            );
        }
    }
}

} // namespace quant::data