#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "bulk/bulk.hpp"

namespace slicerecon::util {

struct bench_listener {
    virtual void bench_notify(std::string name, double time) = 0;
};

struct bencher {
    void insert(std::string name, double time) {
        if (!enabled_) {
            return;
        }

        if (results.find(name) == results.end()) {
            results[name] = {};
        }
        results[name].push_back(time);

        notify_(name, time);
    }

    void print() {
        for (auto [name, times] : results) {
            std::cout << name << ": ";
            for (auto time : times) {
                std::cout << time << " ms, ";
            }
        }
    }

    void register_listener(bench_listener* listener) {
        listeners_.push_back(listener);
        // FIXME remove when, where?
    }

    void notify_(std::string name, double time) {
        for (auto l : listeners_) {
            l->bench_notify(name, time);
        }
    }

    void enable() { enabled_ = true; }

    bool enabled_ = false;
    std::map<std::string, std::vector<double>> results;
    std::vector<bench_listener*> listeners_;
};

extern bencher bench;

struct bench_scope {
    bench_scope(std::string name) : name_(name) {}
    ~bench_scope() { bench.insert(name_, dt.get()); }

    std::string name_;
    bulk::util::timer dt;
};

} // namespace slicerecon::util
