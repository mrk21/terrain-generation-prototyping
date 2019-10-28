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
    T threshold(T v, T t, T l, T u) {
        return v < t ? l : u;
    }

    template<typename T>
    std::string to_string(const T & v) {
        std::ostringstream oss;
        oss << v;
        return oss.str();
    }

    template<typename T>
    std::string to_string(const std::vector<T> & list, const std::string & separator = ", ") {
        std::vector<std::string> string_list;
        std::transform(list.begin(), list.end(), std::back_inserter(string_list), [](const auto & v){ return to_string(v); });
        return boost::algorithm::join(string_list, separator);
    }

    template<typename T>
    void unique(std::vector<T> & v) {
        std::sort(v.begin(), v.end());
        v.erase(std::unique(v.begin(), v.end()), v.end());
    }
}

#endif
