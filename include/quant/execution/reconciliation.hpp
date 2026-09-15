#pragma once

#include "quant/execution/execution_engine.hpp"
#include "quant/execution/portfolio_ledger.hpp"

#include <string>
#include <vector>

namespace quant::execution {

struct ReconciliationIssue {
    std::string symbol;
    std::string message;
};

struct ReconciliationReport {
    bool consistent{};
    std::vector<ReconciliationIssue> issues;
};

[[nodiscard]]
ReconciliationReport reconcile(
    const ExecutionEngine& execution_engine,
    const PortfolioLedger& ledger
);

} // namespace quant::execution