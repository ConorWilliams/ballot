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
    constexpr double bias_fist = 0.95;    // In (0,1)
    constexpr double big_num = 100;       // Sufficiently large
    constexpr double kick_cost = 3;       // Must be less than big_num
    constexpr double p_weight = 1. / 3.;  // Such that >6 terms (2 years) flattens

    /*  */ if (p && r) {
        if (std::optional i = p->choice_index(*r)) {
            // For scaling inverse hyperbolic tangent
            double coef = atanh(bias_fist) / (std::max(1ul, p->pref.size() - 1));
            // Bias hostel choices
            double non_hostel_penalty = is_hostel(r) ? 0.0 : 2 * std::tanh(p->priority * p_weight);

            // Cost of assigning person to room they DO want.  Ensure: 0 < cost <= Kick_cost
            return std::tanh(*i * coef) / bias_fist + non_hostel_penalty;
        } else {
            // Cost of assigning person to room they DO-NOT want, justification:
            //     Bigger than the maximum expected number of players such that it never occurs.
            return big_num;
        }
    } else if (p && !r) {
        // Justification:
        //     Preferable (therefore less than) cost of assigning to unwanted room.
        //     Kicking off ballot should be as close to the cost of getting last choice as this
        //     disincentivises people choosing lots of honey-pot rooms however, this conflicts
        //     with desire to reduce kicking.
        return kick_cost;
    } else {
        // Justification:
        //     Agnostic of room/kicked -> value irrelevant, therefore zero to keep total score
        //     small.
        return 0;
    }
}