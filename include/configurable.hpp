#pragma once


namespace tomovis {

template <typename T>
struct parameter {
    std::string name;
    T min_value;
    T max_value;
    T* value;
};

} // namespace tomovis
