#include "quant/data/alignment.hpp"
#include "quant/data/csv_reader.hpp"
#include "quant/data/validation.hpp"
#include "quant/research/formation.hpp"
#include "quant/research/pairs_research.hpp"
#include "quant/risk/performance.hpp"

#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr
            << "Usage: pairs_research "
            << "<asset_x.csv> <asset_y.csv>\n";

        return 1;
    }

    try {
        const std::string x_filename =
            argv[1];

        const std::string y_filename =
            argv[2];

        /*
         * -----------------------------------------------------
         * 1. Load and validate market data.
         * -----------------------------------------------------
         */

        const auto x =
            quant::data::read_candles_csv(
                x_filename
            );

        const auto y =
            quant::data::read_candles_csv(
                y_filename
            );

        quant::data::validate_market_data(x);
        quant::data::validate_market_data(y);

        const auto aligned =
            quant::data::inner_join(
                x,
                y
            );

        if (aligned.size() < 100) {
            throw std::runtime_error(
                "Too few aligned observations "
                "for research"
            );
        }

        std::cout
            << std::setprecision(10);

        std::cout
            << "Aligned observations: "
            << aligned.size()
            << '\n';

        /*
         * -----------------------------------------------------
         * 2. Formation / trading split.
         * -----------------------------------------------------
         *
         * First 70%:
         *     formation / pair-selection period
         *
         * Remaining 30%:
         *     out-of-sample trading period
         */

        const std::size_t formation_size =
            static_cast<std::size_t>(
                static_cast<double>(
                    aligned.size()
                ) * 0.70
            );

        const auto formation =
            quant::research::run_formation_test(
                aligned,
                formation_size
            );

        std::cout
            << "\n========== FORMATION ==========\n";

        std::cout
            << "Formation observations: "
            << formation.formation_data.size()
            << '\n';

        std::cout
            << "Trading observations:   "
            << formation.trading_data.size()
            << '\n';

        std::cout
            << "OLS intercept:           "
            << formation
                   .cointegration
                   .regression
                   .intercept
            << '\n';

        std::cout
            << "OLS hedge ratio:         "
            << formation
                   .cointegration
                   .regression
                   .slope
            << '\n';

        std::cout
            << "Engle-Granger ADF:       "
            << formation
                   .cointegration
                   .adf_statistic
            << '\n';

        std::cout
            << "MacKinnon 1% critical:   "
            << formation
                   .cointegration
                   .critical_values
                   .one_percent
            << '\n';

        std::cout
            << "MacKinnon 5% critical:   "
            << formation
                   .cointegration
                   .critical_values
                   .five_percent
            << '\n';

        std::cout
            << "MacKinnon 10% critical:  "
            << formation
                   .cointegration
                   .critical_values
                   .ten_percent
            << '\n';

        std::cout
            << "ADF lags:                "
            << formation
                   .cointegration
                   .adf_lags
            << '\n';

        /*
         * -----------------------------------------------------
         * 3. Pair-selection gate.
         * -----------------------------------------------------
         *
         * Absolutely no trading-period performance is
         * calculated when formation fails.
         */

        if (!formation.passes) {
            std::cout
                << "Cointegration decision: FAIL\n";

            std::cout
                << "================================\n";

            std::cout
                << "\nPAIR REJECTED\n";

            std::cout
                << "No out-of-sample backtest "
                << "was performed.\n";

            return 0;
        }

        std::cout
            << "Cointegration decision: PASS\n";

        std::cout
            << "================================\n";

        /*
         * -----------------------------------------------------
         * 4. Strategy parameters.
         * -----------------------------------------------------
         */

        quant::research::PairsResearchParameters
            parameters;

        parameters.hedge_ratio_window = 60;
        parameters.zscore_window = 20;

        parameters.signal_parameters =
            quant::strategy::SignalParameters{
                2.0,
                0.5
            };

        parameters.backtest_parameters =
            quant::backtest::BacktestParameters{
                1'000'000.0,
                100'000.0,
                quant::portfolio::
                    TransactionCostParameters{
                        0.001,
                        0.001
                    }
            };

        /*
         * -----------------------------------------------------
         * 5. Out-of-sample research.
         * -----------------------------------------------------
         *
         * IMPORTANT:
         *
         * We pass the COMPLETE historical series so the first
         * trading observation has legitimate historical
         * lookback for rolling OLS and z-score estimation.
         *
         * However, run_pairs_research() emits/backtests only
         * observations at or after formation_size.
         *
         * All rolling estimators are causal: the model at t
         * uses observations strictly before t.
         */

        const auto research =
            quant::research::run_pairs_research(
                aligned,
                parameters,
                formation_size
            );

        /*
         * -----------------------------------------------------
         * 6. Out-of-sample performance.
         * -----------------------------------------------------
         */

        const auto performance =
            quant::risk::calculate_performance(
                research.backtest,
                252.0
            );

        std::cout
            << "\n======= OUT-OF-SAMPLE =======\n";

        std::cout
            << "Tradable observations:  "
            << research.timestamps.size()
            << '\n';

        std::cout
            << "Initial equity:         "
            << performance.initial_equity
            << '\n';

        std::cout
            << "Final equity:           "
            << performance.final_equity
            << '\n';

        std::cout
            << "Total P&L:              "
            << performance.total_pnl
            << '\n';

        std::cout
            << "Total return:           "
            << performance.total_return * 100.0
            << "%\n";

        std::cout
            << "Annualized return:      "
            << performance.annualized_return * 100.0
            << "%\n";

        std::cout
            << "Annualized volatility:  "
            << performance.annualized_volatility * 100.0
            << "%\n";

        std::cout
            << "Sharpe ratio:           "
            << performance.sharpe_ratio
            << '\n';

        std::cout
            << "Maximum drawdown:       "
            << performance.maximum_drawdown
            << '\n';

        std::cout
            << "Maximum drawdown:       "
            << performance.maximum_drawdown_pct * 100.0
            << "%\n";

        std::cout
            << "Position changes:       "
            << performance.number_of_position_changes
            << '\n';

        std::cout
            << "=============================\n";

        return 0;
    }
    catch (const std::exception& error) {
        std::cerr
            << "Research failed: "
            << error.what()
            << '\n';

        return 1;
    }
}