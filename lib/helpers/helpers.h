#pragma once
#include <algorithm>

class Helpers {

public:

    template <typename T>
    static T between(const T& n, const T& lower, const T& upper) {
        return std::max(lower, std::min(n, upper));
    }
};
