#include "quant/data/csv_reader.hpp"

#include <cassert>
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <string>

int main() {
    const std::string filename =
        "csv_reader_test_data.csv";

    {
        std::ofstream file(filename);

        assert(file);

        file
            << "timestamp,open,high,low,close,volume\n"
            << "1000,10,11,9,10.5,1000\n"
            << "2000,10.5,12,10,11.5,1500\n"
            << "3000,11.5,13,11,12.5,2000\n";
    }

    const auto series =
        quant::data::read_candles_csv(filename);

    assert(series.size() == 3);

    assert(series[0].timestamp == 1000);
    assert(series[1].timestamp == 2000);
    assert(series[2].timestamp == 3000);

    assert(std::abs(
        series[0].open - 10.0
    ) < 1e-12);

    assert(std::abs(
        series[1].close - 11.5
    ) < 1e-12);

    assert(std::abs(
        series[2].volume - 2000.0
    ) < 1e-12);

    /*
     * Invalid header must be rejected.
     */
    const std::string invalid_filename =
        "csv_reader_invalid_header.csv";

    {
        std::ofstream file(invalid_filename);

        assert(file);

        file
            << "bad,header\n"
            << "1000,10,11,9,10.5,1000\n";
    }

    bool threw = false;

    try {
        static_cast<void>(
            quant::data::read_candles_csv(
                invalid_filename
            )
        );
    }
    catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    return 0;
}