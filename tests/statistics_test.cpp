#include "quant/math/statistics.hpp"

#include <cassert>
#include <cmath>

int main() {
    using quant::math::Observation;
    using quant::math::Series;

    Series series;

    series.add(Observation{1000, 1.0});
    series.add(Observation{2000, 2.0});
    series.add(Observation{3000, 3.0});
    series.add(Observation{4000, 4.0});
    series.add(Observation{5000, 5.0});

    const double mu = quant::math::mean(series);
    const double var = quant::math::variance(series);
    const double sd = quant::math::standard_deviation(series);

    assert(std::abs(mu - 3.0) < 1e-12);

    // Sample variance of {1,2,3,4,5} = 2.5
    assert(std::abs(var - 2.5) < 1e-12);

    assert(std::abs(sd - std::sqrt(2.5)) < 1e-12);
}