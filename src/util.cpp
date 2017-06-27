#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>
#include <iostream>

#include "util.hpp"

namespace tomovis {

std::vector<uint32_t> pack(const std::vector<float>& data) {
    std::vector<uint32_t> data_buffer(data.size());

    // convert data to uint32_t, then set
    auto min = *std::min_element(data.begin(), data.end());
    auto max = *std::max_element(data.begin(), data.end());

    auto max_uint = std::numeric_limits<uint32_t>::max() - 1;

    for (auto i = 0u; i < data.size(); ++i) {
        data_buffer[i] = (uint32_t)(max_uint * ((data[i] - min) / max));
    }

    return data_buffer;
}

} // namespace tomovis
