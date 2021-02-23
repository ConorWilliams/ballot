#pragma once

#include <cmath>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>

#include "ballot.hpp"

inline constexpr double cut = 0.99;

struct CostVisitor {
    double operator()(RealPerson const&, RealRoom const&) const { return 0; }
    double operator()(RealPerson const&, Kicked const&) const { return 0; }

    double operator()(AntiPerson const&, RealRoom const&) const { return 0; }
    double operator()(AntiPerson const&, Kicked const&) const { return 0; }

    double operator()(NullPerson const&, RealRoom const&) const { return 0; }
    double operator()(NullPerson const&, Kicked const&) const { return 0; }
};

// Cost function - overall cost is minimised
inline double cost_function(Person const& p, Room const& r) {
    return std::visit(CostVisitor{}, p, r);
    // if (p && r) {
    //     // For scaling inverse hyperbolic tangent
    //     double const coef = atanh(cut) / (std::max(1ul, p->pref.size() - 1));

    //     for (std::size_t i = 0; i < p->pref.size(); i++) {
    //         if (p->pref[i] == *r) {
    //             // Cost of assigning person to room they do want.
    //             // Function guarantees, 0 < cost <= cut
    //             return std::tanh(i * coef);
    //         }
    //     }

    //     // Cost of assigning person to room they do NOT want, justification:
    //     //     Bigger than the maximum expected number of players such that it never occurs.
    //     return 1000;
    // } else if (p && !r) {
    //     // Cost of kicking off ballot, justification:
    //     //     Preferable (therefore less than) to assigning to unwanted room.
    //     //     Kicking off ballot should be as close to the cost of getting last choice as this
    //     //     disincentivise people putting lots of honey-pot rooms however, this conflicts with
    //     //     desire to reduce kicking.
    //     return 1.5;
    // } else if (!p && r) {
    //     // Cost of assigning empty room, justification:
    //     //     Agnostic of room -> value irrelevant, therefore zero to keep total score small.
    //     //     Value must me less than
    //     return 0;
    // } else {
    //     // Cost of binding fake person to empty room, justification:
    //     //     Should only occur if limiting total number of rooms by introducing additional fake
    //     //     people and rooms therefore, cost must be very high as in this scenario we want all
    //     //     excess fake people to bind to real rooms thus this should never preferable.
    //     return 1000;
    // }
}