#include "quant/math/statistics.hpp"

#include <cassert>
#include <cmath>
#include <stdexcept>

int main() {
    using quant::math::Observation;
    using quant::math::Series;

    Series x;
    Series y;

    x.add(Observation{1000, 1.0});
    x.add(Observation{2000, 2.0});
    x.add(Observation{3000, 3.0});
    x.add(Observation{4000, 4.0});

    y.add(Observation{1000, 2.0});
    y.add(Observation{2000, 4.0});
    y.add(Observation{3000, 6.0});
    y.add(Observation{4000, 8.0});

    const double cov =
        quant::math::covariance(x, y);

    const double corr =
        quant::math::correlation(x, y);

    // y = 2x
    // Sample variance(x) = 5/3
    // Cov(x, y) = 2 * 5/3 = 10/3

    assert(std::abs(cov - 10.0 / 3.0) < 1e-12);

    // Perfect positive linear relationship.
    assert(std::abs(corr - 1.0) < 1e-12);

    // Test timestamp alignment.
    Series misaligned;

    misaligned.add(Observation{1000, 2.0});
    misaligned.add(Observation{2001, 4.0});
    misaligned.add(Observation{3000, 6.0});
    misaligned.add(Observation{4000, 8.0});

    bool rejected = false;

    try {
        quant::math::covariance(x, misaligned);
    }
    catch (const std::invalid_argument&) {
        rejected = true;
    }

    assert(rejected);

    // Test zero variance.
    Series constant;

    constant.add(Observation{1000, 5.0});
    constant.add(Observation{2000, 5.0});
    constant.add(Observation{3000, 5.0});

    rejected = false;

    try {
        quant::math::correlation(x, constant);
    }
    catch (const std::domain_error&) {
        rejected = true;
    }

    assert(rejected);
}