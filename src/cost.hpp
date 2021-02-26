// Copyright (C) 2020 Conor Williams

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <cmath>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>

#include "ballot.hpp"

// Cost function - overall cost is minimised
template <typename F> double cost_function(Person const& p, Room const& r, F&& is_hostel) {
    // Constants
    constexpr double bias_fist = 0.95;  // In (0,1)
    constexpr double big_num = 100;
    constexpr double bias_prioity = 1;  // Must be less than big_num

    return match(p, r)(
        [&](RealPerson const& p, RealRoom const& r) -> double {
            // For scaling inverse hyperbolic tangent
            double const coef = atanh(bias_fist) / (std::max(1ul, p.pref.size() - 1));

            for (std::size_t i = 0; i < p.pref.size(); i++) {
                if (p.pref[i] == r) {
                    double non_hostel_penalty = is_hostel(r) ? 0.0 : 0.5;

                    // Cost of assigning person to room they DO want.  Ensure: 0 < cost <= 1
                    return 0.5 * std::tanh(i * coef) / bias_fist + non_hostel_penalty;
                }
            }
            // Cost of assigning person to room they DO-NOT want, justification:
            //     Bigger than the maximum expected number of players such that it never occurs.
            return big_num;
        },
        [](RealPerson const& p, Kicked const&) -> double {
            // Justification:
            //     Preferable (therefore less than) cost of assigning to unwanted room.
            //     Kicking off ballot should be as close to the cost of getting last choice as this
            //     disincentivizes people choosing lots of honey-pot rooms however, this conflicts
            //     with desire to reduce kicking. Functional form gives higher cost to kicking off
            //     lower priority numbers. Guarantees, 1 < cost <= 2
            return 1.0 + bias_prioity * std::exp(-static_cast<double>(p.priority));
        },
        [](AntiPerson const&, RealRoom const&) -> double {
            // Justification:
            //     Should only occur if limiting total number of rooms by introducing additional
            //     anti-people therefore, cost must be 0 as we want all anti-people to bind to rooms
            //     over real-people binding to them.
            return 0;
        },
        [&](AntiPerson const&, Kicked const&) -> double {
            // Justification:
            //     Anti-people must always bind to real-room therefore this must be even more
            //     impossible than the cost of assigning a person to a room they do not want
            return big_num * big_num;
        },
        [](NullPerson const&, auto const&) -> double {
            // Justification:
            //     Agnostic of room/kicked -> value irrelevant, therefore zero to keep total score
            //     small.
            return 0;
        });
}