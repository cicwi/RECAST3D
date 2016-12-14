#include <cstring>

namespace tomovis {

struct scale {
    std::size_t size = 0;

    template <typename T>
    void operator<<(T&) {
        size += sizeof(T);
    }

    void operator<<(std::string& str) {
        size += (str.size() + 1) * sizeof(char);
    }
};

struct memory_buffer {
    memory_buffer(std::size_t size) { buffer = std::make_unique<char[]>(size); }

    memory_buffer(std::size_t size, char* data) {
        buffer = std::make_unique<char[]>(size);
        // double buffer is kind of a waste
        memcpy(buffer.get(), data, size);
    }

    std::unique_ptr<char[]> buffer;
    std::size_t index = 0;

    template <typename T>
    void operator<<(const T& value) {
        memcpy(buffer.get() + index, &value, sizeof(T));
        index += sizeof(T);
    }

    // fix for string
    template <typename T>
    void operator>>(T& value) {
        memcpy(&value, buffer.get() + index, sizeof(T));
        index += sizeof(T);
    }

    // fix for string
    void operator<<(std::string& str) {
        for (auto c : str)
            (*this) << c;
        (*this) << '\0';
    }

    void operator>>(std::string& str) {
        str = std::string(buffer.get() + index);
        index += (str.size() + 1) * sizeof(char);
    }
};

} // namespace tomovis
