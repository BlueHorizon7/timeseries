#include "quant/execution/reconciliation.hpp"

#include <cmath>
#include <unordered_map>

namespace quant::execution {

ReconciliationReport reconcile(
    const ExecutionEngine& execution_engine,
    const PortfolioLedger& ledger)
{
    ReconciliationReport report{};

    std::unordered_map<std::string, double> execution_quantities;

    for (const Order& order : execution_engine.all_orders()) {
        double signed_quantity = 0.0;

        if (order.side == OrderSide::Buy) {
            signed_quantity = order.filled_quantity;
        } else {
            signed_quantity = -order.filled_quantity;
        }

        execution_quantities[order.symbol] += signed_quantity;
    }

    for (const Position& position_value : positions(ledger)) {
        const double execution_quantity =
            execution_quantities[position_value.symbol];

        if (std::abs(
                execution_quantity -
                position_value.quantity) > 1e-9) {

            report.issues.push_back({
                position_value.symbol,
                "ledger position does not match execution fills"
            });
        }
    }

    for (const auto& [symbol, execution_quantity] :
         execution_quantities) {

        static_cast<void>(execution_quantity);

        if (ledger.positions.find(symbol) ==
            ledger.positions.end()) {

            report.issues.push_back({
                symbol,
                "execution history contains symbol absent from ledger"
            });
        }
    }

    report.consistent = report.issues.empty();

    return report;
}

} // namespace quant::execution