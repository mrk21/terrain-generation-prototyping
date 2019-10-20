#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <iostream>
#include <deque>
#include <algorithm>
#include <regex>
#include <functional>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/join.hpp>

namespace Helpers {
    std::string trim_base_indent(const std::string & source);
    void times(uint32_t n, std::function<void(uint32_t)> callback);

    template<typename T>
    T clamp(T v, T l, T u) {
        if (v < l) return l;
        if (v > u) return u;
        return v;
    }

    template<typename T>
    T threshold(T v, T t, T l, T u) {
        return v < t ? l : u;
    }
}

#endif
