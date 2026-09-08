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