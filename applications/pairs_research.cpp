#include "quant/data/alignment.hpp"
#include "quant/data/csv_reader.hpp"
#include "quant/data/validation.hpp"
#include "quant/math/cointegration.hpp"
#include "quant/research/pairs_research.hpp"
#include "quant/risk/performance.hpp"

#include <cmath>
#include <exception>
#include <iomanip>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr
            << "Usage: pairs_research <asset_x.csv> <asset_y.csv>\n";

        return 1;
    }

    try {
        const std::string x_filename = argv[1];
        const std::string y_filename = argv[2];

        /*
         * -----------------------------------------------------
         * 1. Load raw market data.
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

        /*
         * -----------------------------------------------------
         * 2. Validate each market-data series.
         * -----------------------------------------------------
         */
        quant::data::validate_market_data(x);
        quant::data::validate_market_data(y);

        /*
         * -----------------------------------------------------
         * 3. Align timestamps.
         * -----------------------------------------------------
         */
        const auto aligned =
            quant::data::inner_join(x, y);

        if (aligned.size() < 100) {
            throw std::runtime_error(
                "Too few aligned observations for research"
            );
        }

        std::cout
            << "Aligned observations: "
            << aligned.size()
            << '\n';

        /*
         * -----------------------------------------------------
         * 4. Convert aligned data into mathematical Series.
         * -----------------------------------------------------
         */
        quant::math::Series x_series;
        quant::math::Series y_series;

        x_series.reserve(aligned.size());
        y_series.reserve(aligned.size());

        for (const auto& observation : aligned) {
            x_series.add(
                quant::math::Observation{
                    observation.timestamp,
                    observation.x
                }
            );

            y_series.add(
                quant::math::Observation{
                    observation.timestamp,
                    observation.y
                }
            );
        }

        /*
         * -----------------------------------------------------
         * 5. Preliminary Engle-Granger test.
         * -----------------------------------------------------
         *
         * IMPORTANT:
         * The current implementation uses approximate critical
         * values. This is therefore preliminary evidence only.
         */
        const auto cointegration =
            quant::math::engle_granger(
                x_series,
                y_series
            );

        std::cout
            << std::setprecision(10);

        std::cout
            << "OLS intercept: "
            << cointegration.regression.intercept
            << '\n';

        std::cout
            << "OLS hedge ratio: "
            << cointegration.regression.slope
            << '\n';

        std::cout
            << "Engle-Granger ADF statistic: "
            << cointegration.adf_statistic
            << '\n';

        std::cout
            << "Approximate 5% critical value: "
            << cointegration.critical_value_5pct
            << '\n';

        if (cointegration.decision ==
            quant::math::CointegrationDecision::Cointegrated) {

            std::cout
                << "Cointegration decision: "
                << "PASS\n";
        }
        else {
            std::cout
                << "Cointegration decision: "
                << "FAIL\n";
        }

        /*
         * -----------------------------------------------------
         * 6. Configure the trading experiment.
         * -----------------------------------------------------
         *
         * These parameters are intentionally fixed.
         * We are NOT optimizing them against this dataset.
         */
        quant::research::PairsResearchParameters parameters;

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
                quant::portfolio::TransactionCostParameters{
                    0.001,
                    0.001
                }
            };

        /*
         * -----------------------------------------------------
         * 7. Run complete strategy.
         * -----------------------------------------------------
         */
        const auto research =
            quant::research::run_pairs_research(
                aligned,
                parameters
            );

        /*
         * -----------------------------------------------------
         * 8. Calculate performance.
         *
         * Daily data:
         *
         *     252 trading periods/year
         * -----------------------------------------------------
         */
        const auto performance =
            quant::risk::calculate_performance(
                research.backtest,
                252.0
            );

        /*
         * -----------------------------------------------------
         * 9. Report results.
         * -----------------------------------------------------
         */
        std::cout
            << "\n========== PERFORMANCE ==========\n";

        std::cout
            << "Initial equity:        "
            << performance.initial_equity
            << '\n';

        std::cout
            << "Final equity:          "
            << performance.final_equity
            << '\n';

        std::cout
            << "Total P&L:             "
            << performance.total_pnl
            << '\n';

        std::cout
            << "Total return:          "
            << performance.total_return * 100.0
            << "%\n";

        std::cout
            << "Annualized return:     "
            << performance.annualized_return * 100.0
            << "%\n";

        std::cout
            << "Annualized volatility: "
            << performance.annualized_volatility * 100.0
            << "%\n";

        std::cout
            << "Sharpe ratio:          "
            << performance.sharpe_ratio
            << '\n';

        std::cout
            << "Maximum drawdown:      "
            << performance.maximum_drawdown
            << '\n';

        std::cout
            << "Maximum drawdown:      "
            << performance.maximum_drawdown_pct * 100.0
            << "%\n";

        std::cout
            << "Position changes:      "
            << performance.number_of_position_changes
            << '\n';

        std::cout
            << "=================================\n";

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