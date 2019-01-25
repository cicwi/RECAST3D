#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace slicerecon::util {

struct bencher {
    void insert(std::string name, double time) {
        if (results.find(name) == results.end()) {
            results[name] = {};
        }
        results[name].push_back(time);
    }

    void print() {
        for (auto [name, times] : results) {
            std::cout << name << ": ";
            for (auto time : times) {
                std::cout << time << " ms, ";
            }
        }
    }

    ~bencher() { print(); }

    std::map<std::string, std::vector<double>> results;
};

extern bencher bench;

} // namespace slicerecon::util
