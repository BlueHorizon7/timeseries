#include "quant/strategy/pairs_signal.hpp"

#include <cmath>
#include <stdexcept>

namespace quant::strategy {

Position classify_signal(
    double zscore,
    Position current_position,
    const SignalParameters& parameters
) {
    if (!std::isfinite(zscore)) {
        throw std::domain_error(
            "Signal requires a finite z-score"
        );
    }

    if (parameters.entry_zscore <= 0.0) {
        throw std::invalid_argument(
            "Entry z-score must be positive"
        );
    }

    if (parameters.exit_zscore < 0.0 ||
        parameters.exit_zscore >= parameters.entry_zscore) {
        throw std::invalid_argument(
            "Exit z-score must be non-negative and smaller than entry z-score"
        );
    }

    if (zscore <= -parameters.entry_zscore) {
        return Position::LongSpread;
    }

    if (zscore >= parameters.entry_zscore) {
        return Position::ShortSpread;
    }

    if (std::abs(zscore) < parameters.exit_zscore) {
        return Position::Flat;
    }

    return current_position;
}

quant::math::Series generate_signals(
    const quant::math::Series& zscores,
    const SignalParameters& parameters
) {
    quant::math::Series result;

    result.reserve(zscores.size());

    Position current_position =
        Position::Flat;

    for (const auto& observation : zscores) {
        current_position =
            classify_signal(
                observation.value,
                current_position,
                parameters
            );

        result.add(
            quant::math::Observation{
                observation.timestamp,
                static_cast<double>(
                    static_cast<int>(current_position)
                )
            }
        );
    }

    return result;
}

} // namespace quant::strategy