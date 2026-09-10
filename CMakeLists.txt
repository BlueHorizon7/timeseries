cmake_minimum_required(VERSION 3.20)

project(quantlab
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(quantlab
    src/candle.cpp
    src/time_series.cpp
    src/series.cpp
    src/returns.cpp
    src/statistics.cpp
    src/regression.cpp
    src/residuals.cpp
    src/stationarity.cpp
    src/dickey_fuller.cpp
    src/adf.cpp
    src/adf_inference.cpp
    src/unit_root.cpp
    src/cointegration.cpp
    src/rolling_spread.cpp
    src/rolling_regression.cpp
    src/rolling_zscore.cpp
    src/pairs_signal.cpp
    src/pair_position.cpp
    src/pnl.cpp
    src/transaction_cost.cpp
    src/backtest.cpp
    src/performance.cpp
    src/csv_reader.cpp
    src/aligned_series.cpp
    src/alignment.cpp
    src/pairs_research.cpp
    src/validation.cpp
    src/parameter_grid.cpp
    src/trades.cpp
    src/data_validation.cpp
    src/yahoo_csv.cpp
)

target_include_directories(quantlab
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(research
    applications/research/main.cpp
)

target_link_libraries(research
    PRIVATE quantlab
)

enable_testing()

add_executable(candle_test
    tests/candle_test.cpp
)

target_link_libraries(candle_test
    PRIVATE quantlab
)

add_test(
    NAME candle_test
    COMMAND candle_test
)

add_executable(time_series_test
    tests/time_series_test.cpp
)

target_link_libraries(time_series_test
    PRIVATE quantlab
)

add_test(
    NAME time_series_test
    COMMAND time_series_test
)

add_executable(returns_test
    tests/returns_test.cpp
)

target_link_libraries(returns_test
    PRIVATE quantlab
)

add_test(
    NAME returns_test
    COMMAND returns_test
)

add_executable(statistics_test
    tests/statistics_test.cpp
)

target_link_libraries(statistics_test
    PRIVATE quantlab
)

add_test(
    NAME statistics_test
    COMMAND statistics_test
)

add_executable(covariance_test
    tests/covariance_test.cpp
)

target_link_libraries(covariance_test
    PRIVATE quantlab
)

add_test(
    NAME covariance_test
    COMMAND covariance_test
)

add_executable(regression_test
    tests/regression_test.cpp
)

target_link_libraries(regression_test
    PRIVATE quantlab
)

add_test(
    NAME regression_test
    COMMAND regression_test
)

add_executable(stationarity_test
    tests/stationarity_test.cpp
)

target_link_libraries(stationarity_test
    PRIVATE quantlab
)

add_test(
    NAME stationarity_test
    COMMAND stationarity_test
)

add_executable(residuals_test
    tests/residuals_test.cpp
)

target_link_libraries(residuals_test
    PRIVATE quantlab
)

add_test(
    NAME residuals_test
    COMMAND residuals_test
)

add_executable(dickey_fuller_test
    tests/dickey_fuller_test.cpp
)

target_link_libraries(dickey_fuller_test
    PRIVATE quantlab
)

add_test(
    NAME dickey_fuller_test
    COMMAND dickey_fuller_test
)

add_executable(unit_root_test
    tests/unit_root_test.cpp
)

target_link_libraries(unit_root_test
    PRIVATE quantlab
)

add_test(
    NAME unit_root_test
    COMMAND unit_root_test
)

add_executable(adf_test
    tests/adf_test.cpp
)

target_link_libraries(adf_test
    PRIVATE quantlab
)

add_test(
    NAME adf_test
    COMMAND adf_test
)

add_executable(adf_inference_test
    tests/adf_inference_test.cpp
)

target_link_libraries(adf_inference_test
    PRIVATE quantlab
)

add_test(
    NAME adf_inference_test
    COMMAND adf_inference_test
)

add_executable(cointegration_test
    tests/cointegration_test.cpp
)

target_link_libraries(cointegration_test
    PRIVATE quantlab
)

add_test(
    NAME cointegration_test
    COMMAND cointegration_test
)

add_executable(rolling_spread_test
    tests/rolling_spread_test.cpp
)

target_link_libraries(rolling_spread_test
    PRIVATE quantlab
)

add_test(
    NAME rolling_spread_test
    COMMAND rolling_spread_test
)

add_executable(rolling_regression_test
    tests/rolling_regression_test.cpp
)

target_link_libraries(rolling_regression_test
    PRIVATE quantlab
)

add_test(
    NAME rolling_regression_test
    COMMAND rolling_regression_test
)

add_executable(rolling_zscore_test
    tests/rolling_zscore_test.cpp
)

target_link_libraries(rolling_zscore_test
    PRIVATE quantlab
)

add_test(
    NAME rolling_zscore_test
    COMMAND rolling_zscore_test
)

add_executable(pairs_signal_test
    tests/pairs_signal_test.cpp
)

target_link_libraries(pairs_signal_test
    PRIVATE quantlab
)

add_test(
    NAME pairs_signal_test
    COMMAND pairs_signal_test
)

add_executable(pair_position_test
    tests/pair_position_test.cpp
)

target_link_libraries(pair_position_test
    PRIVATE quantlab
)

add_test(
    NAME pair_position_test
    COMMAND pair_position_test
)

add_executable(pnl_test
    tests/pnl_test.cpp
)

target_link_libraries(pnl_test
    PRIVATE quantlab
)

add_test(
    NAME pnl_test
    COMMAND pnl_test
)

add_executable(transaction_cost_test
    tests/transaction_cost_test.cpp
)

target_link_libraries(transaction_cost_test
    PRIVATE quantlab
)

add_test(
    NAME transaction_cost_test
    COMMAND transaction_cost_test
)

add_executable(backtest_test
    tests/backtest_test.cpp
)

target_link_libraries(backtest_test
    PRIVATE quantlab
)

add_test(
    NAME backtest_test
    COMMAND backtest_test
)

add_executable(performance_test
    tests/performance_test.cpp
)

target_link_libraries(performance_test
    PRIVATE quantlab
)

add_test(
    NAME performance_test
    COMMAND performance_test
)

add_executable(csv_reader_test
    tests/csv_reader_test.cpp
)

target_link_libraries(csv_reader_test
    PRIVATE quantlab
)

add_test(
    NAME csv_reader_test
    COMMAND csv_reader_test
)

add_executable(alignment_test
    tests/alignment_test.cpp
)

target_link_libraries(alignment_test
    PRIVATE quantlab
)

add_test(
    NAME alignment_test
    COMMAND alignment_test
)

add_executable(validation_test
    tests/validation_test.cpp
)

target_link_libraries(validation_test
    PRIVATE quantlab
)

add_test(
    NAME validation_test
    COMMAND validation_test
)

add_executable(parameter_grid_test
    tests/parameter_grid_test.cpp
)

target_link_libraries(parameter_grid_test
    PRIVATE quantlab
)

add_test(
    NAME parameter_grid_test
    COMMAND parameter_grid_test
)

add_executable(trades_test
    tests/trades_test.cpp
)

target_link_libraries(trades_test
    PRIVATE quantlab
)

add_test(
    NAME trades_test
    COMMAND trades_test
)

add_executable(data_validation_test
    tests/data_validation_test.cpp
)

target_link_libraries(data_validation_test
    PRIVATE quantlab
)

add_test(
    NAME data_validation_test
    COMMAND data_validation_test
)

add_executable(pairs_research
    applications/pairs_research.cpp
)

target_link_libraries(pairs_research
    PRIVATE quantlab
)