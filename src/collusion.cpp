// Copyright (C) 2020 Conor Williams

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "collusion.hpp"

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>

#include "ballot.hpp"

void report_k_cycles(std::size_t k, std::vector<Person> const& people) {
    //
    struct AugPerson : impl::Person {
        std::set<std::string> sub_pref;
    };

    std::vector<AugPerson> first_k;

    for (auto&& p : people) {
        if (p) {
            first_k.push_back({
                *p,
                {p->pref.begin(), p->pref.begin() + std::min(k, p->pref.size())},
            });
        }
    }

    // Find longest name for pretty print
    std::size_t w = 0;

    for (auto&& p : first_k) {
        w = std::max(w, p.name.size());
    }

    for (auto it = first_k.begin(); it != first_k.end();) {
        //
        auto end = std::partition(it, first_k.end(), [s = *it](auto const& x) {  //
            return x.sub_pref == s.sub_pref;
        });

        if (end - it >= static_cast<std::ptrdiff_t>(k)) {
            // Report choices
            for (auto ix = it; ix != end; ++ix) {
                std::cout << "--" << std::right << std::setw(w + 2) << ix->name << " : ";
                for (std::size_t i = 0; i < k; i++) {
                    std::cout << std::left << std::setw(5) << ix->pref[i];
                }

                std::cout << '\n';
            }
            std::cout << "--\n";
        }

        it = end;
    }
}