#include "quant/data/csv_reader.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace quant::data {

namespace {

std::int64_t parse_timestamp(
    const std::string& text
) {
    // Unix timestamp.
    if (text.find('-') == std::string::npos) {
        std::size_t position = 0;

        const auto timestamp =
            std::stoll(text, &position);

        if (position != text.size()) {
            throw std::invalid_argument(
                "Invalid timestamp: " + text
            );
        }

        return timestamp;
    }

    // Date format:
    // YYYY-MM-DD
    std::tm tm{};

    std::istringstream stream(text);

    stream >> std::get_time(
        &tm,
        "%Y-%m-%d"
    );

    if (stream.fail()) {
        throw std::invalid_argument(
            "Invalid date: " + text
        );
    }

    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;

#ifdef _WIN32
    return static_cast<std::int64_t>(
        _mkgmtime(&tm)
    );
#else
    return static_cast<std::int64_t>(
        timegm(&tm)
    );
#endif
}

double parse_double(
    const std::string& text,
    const std::string& field
) {
    std::size_t position = 0;

    const double value =
        std::stod(text, &position);

    if (position != text.size()) {
        throw std::invalid_argument(
            "Invalid " + field + ": " + text
        );
    }

    return value;
}

} // namespace

TimeSeries read_candles_csv(
    const std::string& filename
) {
    std::ifstream input(filename);

    if (!input) {
        throw std::runtime_error(
            "Unable to open CSV: " + filename
        );
    }

    std::string line;

    if (!std::getline(input, line)) {
        throw std::invalid_argument(
            "CSV file is empty"
        );
    }

    /*
     * QuantLab accepts both:
     *
     * timestamp,open,high,low,close,volume
     *
     * and the format produced by the yfinance
     * acquisition pipeline:
     *
     * Date,Open,High,Low,Close,Volume
     */
    const bool canonical_header =
        line ==
        "timestamp,open,high,low,close,volume";

    const bool yfinance_header =
        line ==
        "Date,Open,High,Low,Close,Volume";

    if (!canonical_header &&
        !yfinance_header) {

        throw std::invalid_argument(
            "Unexpected CSV header in: " +
            filename
        );
    }

    std::vector<Candle> rows;

    std::size_t line_number = 1;

    while (std::getline(input, line)) {
        ++line_number;

        if (line.empty()) {
            continue;
        }

        std::stringstream stream(line);

        std::string timestamp;
        std::string open;
        std::string high;
        std::string low;
        std::string close;
        std::string volume;

        if (!std::getline(stream, timestamp, ',') ||
            !std::getline(stream, open, ',') ||
            !std::getline(stream, high, ',') ||
            !std::getline(stream, low, ',') ||
            !std::getline(stream, close, ',') ||
            !std::getline(stream, volume)) {

            throw std::invalid_argument(
                "Malformed CSV at line " +
                std::to_string(line_number)
            );
        }

        try {
            rows.push_back(
                Candle{
                    parse_timestamp(timestamp),
                    parse_double(open, "open"),
                    parse_double(high, "high"),
                    parse_double(low, "low"),
                    parse_double(close, "close"),
                    parse_double(volume, "volume")
                }
            );
        }
        catch (const std::exception& error) {
            throw std::invalid_argument(
                "Invalid CSV at line " +
                std::to_string(line_number) +
                ": " +
                error.what()
            );
        }
    }

    if (rows.empty()) {
        throw std::invalid_argument(
            "CSV contains no observations"
        );
    }

    /*
     * Some providers return newest -> oldest.
     *
     * QuantLab requires:
     *
     * t1 < t2 < ... < tn
     */
    std::sort(
        rows.begin(),
        rows.end(),
        [](const Candle& lhs,
           const Candle& rhs) {

            return lhs.timestamp <
                   rhs.timestamp;
        }
    );

    TimeSeries result;

    result.reserve(rows.size());

    for (const auto& candle : rows) {
        result.add(candle);
    }

    return result;
}

} // namespace quant::data