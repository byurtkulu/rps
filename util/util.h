//
// Created by Bahadır Yurtkulu on 26/02/2023.
//

#pragma once
#include <cstdint>

namespace rps::util {
    class Counter {
    public:
        Counter() = default;
        Counter(int64_t c) : c_(c) {}

        int64_t next() {
            return ++c_;
        }

        int64_t current() const {
            return c_;
        }

    private:
        int64_t c_ = 0;
    };
}
