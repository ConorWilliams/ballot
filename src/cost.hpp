#pragma once

#include <cmath>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include "ballot.hpp"

inline constexpr double cut = 0.99;

// Cost function - overall cost is minimised
inline double cost_function(std::optional<Person> const& p, std::optional<std::string> const& r) {
    if (p && r) {
        // For scaling inverse hyperbolic tangent
        double const coef = atanh(cut) / (std::max(1ul, p->pref.size() - 1));

        for (std::size_t i = 0; i < p->pref.size(); i++) {
            if (p->pref[i] == *r) {
                // Cost of assigning person to room they do want.
                // Function guarantees, 0 < cost <= cut
                return std::tanh(i * coef);
            }
        }

        // Cost of assigning person to room they do NOT want, justification:
        //  Bigger than the maximum number expected number of players such that it never occurs
        return 500;
    } else if (p && !r) {
        // Cost of kicking off ballot, justification:
        // Preferable (therefore less than) to assigning to unwanted room
        //  Kicking off ballot should be as close to the cost of getting last choice as this
        //  disincentivise people putting lots of honey-pot rooms however, this conficts with desire
        //  to reduce kicking,
        return 1.5;
    } else if (!p && r) {
        // Cost of assigning empty room, justification:
        //  Agnostic of room -> value irrelevant, therefore set to zero to keep total score small
        return 0;
    } else {
        // Should never occur
        throw std::runtime_error("Both people and rooms are padded!");
    }
}