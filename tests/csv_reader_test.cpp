#include "quant/data/csv_reader.hpp"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>

int main() {
    const std::string filename =
        "csv_reader_test.csv";

    {
        std::ofstream output(filename);

        assert(output);

        output
            << "timestamp,open,high,low,close,volume\n";

        output
            << "2026-09-09,88.31,88.40,87.41,87.55,12337500\n";

        output
            << "2026-09-08,87.55,88.55,87.55,88.36,19426900\n";

        output
            << "2026-09-04,88.59,88.91,87.85,88.07,17290700\n";
    }

    const auto series =
        quant::data::read_candles_csv(filename);

    assert(series.size() == 3);

    // Reader must normalize descending input
    // into ascending chronological order.
    assert(
        series[0].timestamp <
        series[1].timestamp
    );

    assert(
        series[1].timestamp <
        series[2].timestamp
    );

    // Earliest observation.
    assert(
        series[0].close == 88.07
    );

    // Latest observation.
    assert(
        series[2].close == 87.55
    );

    assert(
        series[2].volume == 12337500.0
    );

    std::remove(filename.c_str());

    return 0;
}