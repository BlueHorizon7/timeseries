#pragma once

#include "quant/execution/fill.hpp"
#include "quant/execution/order.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace quant::execution {

struct SimulatedVenueParameters {
    double buy_slippage_bps{};
    double sell_slippage_bps{};
    double commission_rate{};

    double partial_fill_fraction{1.0};

    std::int64_t latency{};
};

class SimulatedExecutionVenue {
public:
    explicit SimulatedExecutionVenue(
        SimulatedVenueParameters parameters = {}
    );

    [[nodiscard]]
    std::vector<Fill> execute(
        const Order& order,
        double market_price,
        std::int64_t market_timestamp
    ) const;

private:
    SimulatedVenueParameters parameters_;
};

} // namespace quant::execution