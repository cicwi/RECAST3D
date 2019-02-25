#pragma once

#include <cstring>
#include <iostream>
#include <memory>

namespace tomop {

struct scale {
    std::size_t size = 0;

    template <typename T>
    void operator|(T&) {
        size += sizeof(T);
    }

    void operator|(std::string& str) {
        size += (str.size() + 1) * sizeof(char);
    }

    template <typename T>
    void operator|(std::vector<T>& xs) {
        size += sizeof(int);
        for (auto x : xs) {
            (*this) | x;
        }
    }
};

struct memory_span {
    // TODO: Add size, and asserts with size checks
    memory_span(std::size_t, char* data_) : data(data_) {}
    memory_span() : data(nullptr) {}

    char* data = nullptr;
    std::size_t index = 0;

    template <typename T>
    void operator<<(const T& value) {
        memcpy(data + index, &value, sizeof(T));
        index += sizeof(T);
    }

    template <typename T>
    void operator>>(T& value) {
        memcpy(&value, data + index, sizeof(T));
        index += sizeof(T);
    }

    void operator<<(std::string& str) {
        for (auto c : str) {
            (*this) << c;
        }
        (*this) << '\0';
    }

    void operator>>(std::string& str) {
        str = std::string(data + index);
        index += (str.size() + 1) * sizeof(char);
    }

    template <typename T>
    void operator<<(std::vector<T>& xs) {
        (*this) << (int)xs.size();
        for (auto& x : xs) {
            (*this) << x;
        }
    }

    template <typename T>
    void operator>>(std::vector<T>& xs) {
        int size = 0;
        (*this) >> size;
        xs.resize(size);
        for (auto& x : xs) {
            (*this) >> x;
        }
    }
};

struct memory_buffer : public memory_span {
    memory_buffer(std::size_t size) {
        buffer = std::make_unique<char[]>(size);
        data = buffer.get();
    }

    memory_buffer(std::size_t size, char* data_) {
        buffer = std::make_unique<char[]>(size);
        // double buffer is kind of a waste
        memcpy(buffer.get(), data_, size);
        data = buffer.get();
    }

    std::unique_ptr<char[]> buffer;
};

} // namespace tomop
