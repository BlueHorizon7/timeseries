#pragma once

#include <string>

namespace quant::data {

void normalize_yahoo_csv(
    const std::string& input_filename,
    const std::string& output_filename
);

} // namespace quant::data