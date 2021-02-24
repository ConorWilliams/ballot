#include "collusion.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>

#include "ballot.hpp"

namespace {

struct SubPerson : RealPerson {
    std::set<RealRoom> sub_pref;
};

}  // namespace

void report_k_cycles(std::size_t k, std::vector<RealPerson> const& people) {
    //
    std::vector<SubPerson> first_k;

    for (auto&& p : people) {
        first_k.push_back({p, {p.pref.begin(), p.pref.begin() + std::min(k, p.pref.size())}});
    }

    std::cout << "-- Scanning for " << k << "-cycles:\n";

    for (auto it = first_k.begin(); it != first_k.end();) {
        auto nx = std::partition(it, first_k.end(), [s = it->sub_pref](auto const& x) {  //
            return x.sub_pref == s;
        });

        if (nx - it > 1) {
            // Find longest name for pretty print
            std::size_t w = 0;

            for (auto ix = it; ix != nx; ++ix) {
                w = std::max(w, nx->name.size());
            }

            // Report all choices
            for (auto ix = it; ix != nx; ++ix) {
                std::cout << "--" << std::right << std::setw(w + 4) << ix->name << " : ";
                for (auto&& room : ix->pref) {
                    std::cout << std::left << std::setw(5) << room;
                }
                std::cout << '\n';
            }
            std::cout << "--\n";
        }

        it = nx;
    }
}