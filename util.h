//
// Created by Bahadır Yurtkulu on 26/02/2023.
//

#pragma once
#include <cstdint>

namespace rps::util {
    class Counter {
    public:
        Counter() = default;
        Counter(uint64_t c) : c_(c) {}

        uint64_t next() {
            return ++c_;
        }

        uint64_t current() const {
            return c_;
        }

    private:
        uint64_t c_ = 0;
    };
}
