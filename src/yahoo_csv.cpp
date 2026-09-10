#include "quant/data/yahoo_csv.hpp"

#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <cstdint>

namespace quant::data {

namespace {

std::int64_t date_to_timestamp(
    const std::string& date
) {
    std::tm tm{};

    std::istringstream stream(date);

    stream >> std::get_time(
        &tm,
        "%Y-%m-%d"
    );

    if (stream.fail()) {
        throw std::invalid_argument(
            "Invalid Yahoo date: " + date
        );
    }

    /*
     * Historical daily data is interpreted as UTC midnight.
     *
     * This is sufficient for daily research because both
     * instruments use the same date convention.
     */
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

} // namespace

void normalize_yahoo_csv(
    const std::string& input_filename,
    const std::string& output_filename
) {
    std::ifstream input(input_filename);

    if (!input) {
        throw std::runtime_error(
            "Unable to open Yahoo CSV: " +
            input_filename
        );
    }

    std::ofstream output(output_filename);

    if (!output) {
        throw std::runtime_error(
            "Unable to create output CSV: " +
            output_filename
        );
    }

    std::string line;

    if (!std::getline(input, line)) {
        throw std::invalid_argument(
            "Yahoo CSV is empty"
        );
    }

    output
        << "timestamp,open,high,low,close,volume\n";

    std::size_t line_number = 1;

    while (std::getline(input, line)) {
        ++line_number;

        if (line.empty()) {
            continue;
        }

        std::stringstream stream(line);

        std::string date;
        std::string open;
        std::string high;
        std::string low;
        std::string close;
        std::string adjusted_close;
        std::string volume;

        if (!std::getline(stream, date, ',') ||
            !std::getline(stream, open, ',') ||
            !std::getline(stream, high, ',') ||
            !std::getline(stream, low, ',') ||
            !std::getline(stream, close, ',') ||
            !std::getline(stream, adjusted_close, ',') ||
            !std::getline(stream, volume, ',')) {

            throw std::invalid_argument(
                "Malformed Yahoo CSV at line " +
                std::to_string(line_number)
            );
        }

        const auto timestamp =
            date_to_timestamp(date);

        output
            << timestamp << ','
            << open << ','
            << high << ','
            << low << ','
            << close << ','
            << volume << '\n';
    }
}

} // namespace quant::data
