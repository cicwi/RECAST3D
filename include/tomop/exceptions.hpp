#pragma once

#include <stdexcept>

namespace tomop {

class server_error : public std::runtime_error {
    using runtime_error::runtime_error;
};

} // namespace tomop
