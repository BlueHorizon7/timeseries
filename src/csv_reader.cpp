#include "quant/data/csv_reader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace quant::data {

namespace {

double parse_double(
    const std::string& value,
    const std::string& field,
    std::size_t line_number
) {
    try {
        std::size_t position = 0;

        const double result =
            std::stod(value, &position);

        if (position != value.size()) {
            throw std::invalid_argument(
                "Trailing characters"
            );
        }

        return result;
    }
    catch (const std::exception&) {
        throw std::invalid_argument(
            "Invalid " + field +
            " at CSV line " +
            std::to_string(line_number)
        );
    }
}

std::int64_t parse_timestamp(
    const std::string& value,
    std::size_t line_number
) {
    try {
        std::size_t position = 0;

        const auto result =
            std::stoll(value, &position);

        if (position != value.size()) {
            throw std::invalid_argument(
                "Trailing characters"
            );
        }

        return result;
    }
    catch (const std::exception&) {
        throw std::invalid_argument(
            "Invalid timestamp at CSV line " +
            std::to_string(line_number)
        );
    }
}

} // namespace

TimeSeries read_candles_csv(
    const std::string& filename
) {
    std::ifstream file(filename);

    if (!file) {
        throw std::runtime_error(
            "Unable to open CSV file: " + filename
        );
    }

    TimeSeries result;

    std::string line;
    std::size_t line_number = 0;

    /*
     * Read header.
     */
    if (!std::getline(file, line)) {
        throw std::invalid_argument(
            "CSV file is empty"
        );
    }

    ++line_number;

    const std::string expected_header =
        "timestamp,open,high,low,close,volume";

    if (line != expected_header) {
        throw std::invalid_argument(
            "Unexpected CSV header"
        );
    }

    while (std::getline(file, line)) {
        ++line_number;

        if (line.empty()) {
            continue;
        }

        std::stringstream stream(line);
        std::string field;

        std::vector<std::string> fields;

        while (std::getline(stream, field, ',')) {
            fields.push_back(field);
        }

        if (fields.size() != 6) {
            throw std::invalid_argument(
                "Expected 6 fields at CSV line " +
                std::to_string(line_number)
            );
        }

        const std::int64_t timestamp =
            parse_timestamp(
                fields[0],
                line_number
            );

        const double open =
            parse_double(
                fields[1],
                "open",
                line_number
            );

        const double high =
            parse_double(
                fields[2],
                "high",
                line_number
            );

        const double low =
            parse_double(
                fields[3],
                "low",
                line_number
            );

        const double close =
            parse_double(
                fields[4],
                "close",
                line_number
            );

        const double volume =
            parse_double(
                fields[5],
                "volume",
                line_number
            );

        result.add(
            Candle{
                timestamp,
                open,
                high,
                low,
                close,
                volume
            }
        );
    }

    return result;
}

} // namespace quant::data