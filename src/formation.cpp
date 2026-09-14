#include "quant/research/formation.hpp"

#include "quant/math/observation.hpp"
#include "quant/math/series.hpp"

#include <stdexcept>

namespace quant::research {

namespace {

quant::math::Series make_x_series(
    const quant::data::AlignedSeries& data
) {
    quant::math::Series result;

    result.reserve(data.size());

    for (const auto& observation : data) {
        result.add(
            quant::math::Observation{
                observation.timestamp,
                observation.x
            }
        );
    }

    return result;
}

quant::math::Series make_y_series(
    const quant::data::AlignedSeries& data
) {
    quant::math::Series result;

    result.reserve(data.size());

    for (const auto& observation : data) {
        result.add(
            quant::math::Observation{
                observation.timestamp,
                observation.y
            }
        );
    }

    return result;
}

quant::data::AlignedSeries slice(
    const quant::data::AlignedSeries& data,
    std::size_t begin,
    std::size_t end
) {
    if (begin > end ||
        end > data.size()) {

        throw std::invalid_argument(
            "Invalid aligned-series slice"
        );
    }

    quant::data::AlignedSeries result;

    result.reserve(end - begin);

    for (std::size_t i = begin;
         i < end;
         ++i) {

        result.add(data[i]);
    }

    return result;
}

} // namespace

FormationResult run_formation_test(
    const quant::data::AlignedSeries& data,
    std::size_t formation_size
) {
    const FormationParameters parameters{
        formation_size,
        quant::math::CointegrationParameters{
            false,
            0,
            0,
            quant::math::InformationCriterion::AIC
        }
    };

    return run_formation_test(
        data,
        parameters
    );
}

FormationResult run_formation_test(
    const quant::data::AlignedSeries& data,
    const FormationParameters&
        parameters
) {
    if (data.size() < 20) {
        throw std::invalid_argument(
            "Formation test requires at least "
            "20 observations"
        );
    }

    if (parameters.formation_size < 20) {
        throw std::invalid_argument(
            "Formation period must contain at least "
            "20 observations"
        );
    }

    if (parameters.formation_size >= data.size()) {
        throw std::invalid_argument(
            "Formation period must leave observations "
            "for trading"
        );
    }

    const auto formation_data =
        slice(
            data,
            0,
            parameters.formation_size
        );

    const auto trading_data =
        slice(
            data,
            parameters.formation_size,
            data.size()
        );

    const auto x =
        make_x_series(
            formation_data
        );

    const auto y =
        make_y_series(
            formation_data
        );

    const auto cointegration =
        quant::math::engle_granger(
            x,
            y,
            parameters.cointegration
        );

    const bool passes =
        cointegration.decision ==
        quant::math::CointegrationDecision::
            Cointegrated;

    return FormationResult{
        formation_data,
        trading_data,
        cointegration,
        passes
    };
}

} // namespace quant::research