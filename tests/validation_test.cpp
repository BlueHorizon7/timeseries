#include "quant/research/validation.hpp"

#include <cassert>

int main() {
    using namespace quant::data;
    using namespace quant::research;

    AlignedSeries data;

    for (std::size_t i = 0; i < 20; ++i) {
        data.add(
            AlignedObservation{
                static_cast<std::int64_t>(1000 + i),
                static_cast<double>(i),
                static_cast<double>(2 * i)
            }
        );
    }

    /*
     * 70% training / 30% testing.
     */
    const auto split =
        train_test_split(data, 0.70);

    assert(split.training.size() == 14);
    assert(split.testing.size() == 6);

    assert(
        split.training[0].timestamp == 1000
    );

    assert(
        split.training[13].timestamp == 1013
    );

    assert(
        split.testing[0].timestamp == 1014
    );

    assert(
        split.testing[5].timestamp == 1019
    );

    /*
     * Expanding-window validation.
     *
     * Training = 10
     * Testing  = 4
     * Step     = 4
     */
    const auto windows =
        make_walk_forward_windows(
            20,
            10,
            4,
            4
        );

    assert(windows.size() == 2);

    assert(windows[0].training_begin == 0);
    assert(windows[0].training_end == 10);
    assert(windows[0].testing_begin == 10);
    assert(windows[0].testing_end == 14);

    assert(windows[1].training_begin == 0);
    assert(windows[1].training_end == 14);
    assert(windows[1].testing_begin == 14);
    assert(windows[1].testing_end == 18);

    return 0;
}