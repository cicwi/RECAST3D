#pragma once

#include <stdexcept>

namespace slicerecon {

class server_error : public std::runtime_error {
    using runtime_error::runtime_error;
};

} // namespace slicerecon
