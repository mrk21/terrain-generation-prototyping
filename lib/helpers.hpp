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
    std::string trim_base_indent(const std::string & source) {
        std::deque<std::string> lines;
        boost::split(lines, source, boost::is_any_of("\n"));
        lines.pop_front();
        lines.pop_back();

        uint32_t min = 9999;
        std::for_each(lines.begin(), lines.end(), [&min](const auto & a){
            if (a.size() == 0) return;
            std::regex re(R"(^ +)");
            std::smatch m;
            auto c = static_cast<uint32_t>(std::regex_search(a, m, re) ? m.str().size() : 0);
            min = std::min(min, c);
        });
        std::for_each(lines.begin(), lines.end(), [&min](auto & a){
            if (a.size() == 0) return;
            auto aa = a.substr(min);
            a.swap(aa);
        });
        return boost::algorithm::join(lines, "\n");
    }

    void times(uint32_t n, std::function<void(uint32_t)> callback) {
        for (uint32_t i = 0; i < n; ++i) {
            callback(i);
        }
    }

    template<typename T>
    double clamp(T v, T l, T u) {
        if (v < l) return l;
        if (v > u) return u;
        return v;
    }

    template<typename T>
    double threshold(T v, T t, T l, T u) {
        return v < t ? l : u;
    }
}

#endif
