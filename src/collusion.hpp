#pragma once

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>

#include "ballot.hpp"

inline std::vector<std::set<RealRoom>> choices(std::vector<RealPerson> const& people) {
    std::vector<std::set<RealRoom>> out;

    for (auto&& p : people) {
        out.emplace_back(p.pref.begin(), p.pref.end());
    }

    return out;
}

template <typename T> std::set<T> intersection(std::set<T> a, std::set<T> b) {
    std::set<T> tmp;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::inserter(tmp, tmp.begin()));
    return tmp;
}

inline void find_collusion(std::vector<RealPerson> const& people) {
    auto k2 = choices(people);

    std::cout << "-- Suspected cheaters:\n";

    for (std::size_t i = 0; i < k2.size(); i++) {
        std::size_t count = 0;

        for (std::size_t j = i + 1; j < k2.size(); j++) {
            auto inter = intersection(k2[i], k2[j]);

            if (inter.size() >= 6) {
                ++count;
                auto w = std::max(people[i].name.size(), people[j].name.size()) + 4;

                std::cout << "--" << std::setw(w) << people[j].name << ':';
                for (auto&& room : people[j].pref) {
                    std::cout << ' ' << room;
                }
                std::cout << '\n';
            }
        }

        if (count > 0 && i + 1 < k2.size()) {
            std::cout << "--" << std::setw(10) << people[i].name << ':';
            for (auto&& room : people[i].pref) {
                std::cout << ' ' << room;
            }
            std::cout << '\n';

            std::cout << "-- Suspected cheaters:\n";
        }
    }
}