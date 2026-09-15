#include "quant/execution/execution_coordinator.hpp"
#include "quant/execution/paper_trader.hpp"
#include "quant/execution/reconciliation.hpp"
#include "quant/execution/simulated_venue.hpp"

#include <iostream>
#include <vector>

int main()
{
    using namespace quant::execution;

    ExecutionEngine execution_engine;

    PortfolioLedger ledger{};
    ledger.cash = 100000.0;

    SimulatedExecutionVenue venue({
        .buy_slippage_bps = 1.0,
        .sell_slippage_bps = 1.0,
        .commission_rate = 0.0005,
        .partial_fill_fraction = 1.0,
        .latency = 0
    });

    ExecutionCoordinator coordinator(
        execution_engine,
        ledger,
        venue
    );

    PaperTrader trader(
        ledger,
        coordinator
    );

    const std::vector<std::int64_t> timestamps{
        1, 2, 3, 4, 5
    };

    const std::vector<
        std::unordered_map<std::string, double>> prices{
        {{"KO", 70.0}, {"PEP", 170.0}},
        {{"KO", 71.0}, {"PEP", 169.0}},
        {{"KO", 72.0}, {"PEP", 168.0}},
        {{"KO", 71.0}, {"PEP", 169.0}},
        {{"KO", 70.0}, {"PEP", 170.0}}
    };

    const std::vector<std::vector<TargetPosition>> targets{
        {
            {"KO", 100.0},
            {"PEP", -41.0}
        },
        {
            {"KO", 100.0},
            {"PEP", -41.0}
        },
        {
            {"KO", 100.0},
            {"PEP", -41.0}
        },
        {
            {"KO", 0.0},
            {"PEP", 0.0}
        },
        {
            {"KO", 0.0},
            {"PEP", 0.0}
        }
    };

    const auto result =
        trader.run(
            timestamps,
            prices,
            targets
        );

    const auto reconciliation =
        reconcile(
            execution_engine,
            ledger
        );

    std::cout
        << "Bars: "
        << result.bars.size()
        << '\n';

    std::cout
        << "Cash: "
        << ledger.cash
        << '\n';

    std::cout
        << "Realized PnL: "
        << ledger.realized_pnl
        << '\n';

    std::cout
        << "Commission: "
        << ledger.total_commission
        << '\n';

    std::cout
        << "Reconciled: "
        << (reconciliation.consistent ? "yes" : "no")
        << '\n';

    return reconciliation.consistent ? 0 : 1;
}