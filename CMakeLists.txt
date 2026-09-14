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
    src/mackinnon.cpp
    src/formation.cpp
    src/walk_forward.cpp
    src/adf_selection.cpp
    src/pair_universe.cpp
    src/multiple_testing.cpp
        src/pair_selection.cpp
        src/multi_pair_allocation.cpp
        src/covariance_matrix.cpp
        src/portfolio_risk_limits.cpp
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

add_executable(mackinnon_test
    tests/mackinnon_test.cpp
)

target_link_libraries(mackinnon_test
    PRIVATE quantlab
)

add_test(
    NAME mackinnon_test
    COMMAND mackinnon_test
)

add_executable(
    formation_test
    tests/formation_test.cpp
)

target_link_libraries(
    formation_test
    PRIVATE quantlab
)

add_test(
    NAME formation_test
    COMMAND formation_test
)

add_executable(test_walk_forward_aggregation tests/test_walk_forward_aggregation.cpp)
target_link_libraries(test_walk_forward_aggregation PRIVATE quantlab) # Ensure 'quantlab' is your library target name
add_test(NAME test_walk_forward_aggregation COMMAND test_walk_forward_aggregation)

add_executable(
    adf_selection_test
    tests/adf_selection_test.cpp
)

target_link_libraries(
    adf_selection_test
    PRIVATE quantlab
)

add_test(
    NAME adf_selection_test
    COMMAND adf_selection_test
)

add_executable(
    pair_universe_test
    tests/pair_universe_test.cpp
)

target_link_libraries(
    pair_universe_test
    PRIVATE
    quantlab
)

add_test(
    NAME pair_universe_test
    COMMAND pair_universe_test
)

add_executable(
    multiple_testing_test
    tests/multiple_testing_test.cpp
)

target_link_libraries(
    multiple_testing_test
    PRIVATE
    quantlab
)

add_test(
    NAME multiple_testing_test
    COMMAND multiple_testing_test
)

add_executable(mackinnon_p_value_test
    tests/mackinnon_p_value_test.cpp
)

target_include_directories(mackinnon_p_value_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(mackinnon_p_value_test
    PRIVATE
        quantlab
)

add_test(
    NAME mackinnon_p_value_test
    COMMAND mackinnon_p_value_test
)

add_executable(pair_selection_test
    tests/pair_selection_test.cpp
)

target_include_directories(pair_selection_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(pair_selection_test
    PRIVATE
        quantlab
)

add_test(
    NAME pair_selection_test
    COMMAND pair_selection_test
)

add_executable(multi_pair_allocation_test
    tests/multi_pair_allocation_test.cpp
)

target_include_directories(multi_pair_allocation_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(multi_pair_allocation_test
    PRIVATE
        quantlab
)

add_test(
    NAME multi_pair_allocation_test
    COMMAND multi_pair_allocation_test
)

add_executable(
    covariance_matrix_test
    tests/covariance_matrix_test.cpp
)

target_include_directories(
    covariance_matrix_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    covariance_matrix_test
    PRIVATE
        quantlab
)

add_test(
    NAME covariance_matrix_test
    COMMAND covariance_matrix_test
)

add_executable(
    portfolio_risk_limits_test
    tests/portfolio_risk_limits_test.cpp
)

target_include_directories(
    portfolio_risk_limits_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    portfolio_risk_limits_test
    PRIVATE
        quantlab
)

add_test(
    NAME portfolio_risk_limits_test
    COMMAND portfolio_risk_limits_test
)