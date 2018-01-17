#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <vector>

#include "util.hpp"

namespace tomovis {

std::vector<uint32_t> pack(const std::vector<float> &data,
                           float min_value, float max_value) {
  std::vector<uint32_t> data_buffer(data.size());

  // convert data to uint32_t, then set
  auto min = min_value;
  auto max = max_value;
  if (max < min) {
    min = *std::min_element(data.begin(), data.end());
    max = *std::max_element(data.begin(), data.end());
  }

  auto max_uint = std::numeric_limits<uint32_t>::max() - 128;

  for (auto i = 0u; i < data.size(); ++i) {
    data_buffer[i] = (uint32_t)(max_uint * ((data[i] - min) / (max - min)));
  }

  return data_buffer;
}

} // namespace tomovis
