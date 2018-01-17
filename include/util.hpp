#pragma once

#include <cstddef>
#include <vector>

namespace tomovis {

std::vector<uint32_t> pack(const std::vector<float>& data,
                           float min_value = 1.0f, float max_value = -1.0f);

} // namespace tomovis
