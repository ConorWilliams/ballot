#pragma once

#include <limits>
#include <optional>
#include <stdexcept>
#include <string>

#include "ballot.hpp"

// Cost function - overall cost is minimised
inline double cost_function(std::optional<Person> const& p, std::optional<std::string> const& r) {
    if (p && r) {
        // Cost of assigning person to room they do want
        for (std::size_t i = 0; i < p->pref.size(); i++) {
            if (p->pref[i] == *r) {
                return i;
            }
        }

        // Cost of assigning person to room they do NOT want
        return 100000;
    } else if (p && !r) {
        // Cost of kicking off ballot, preferable to assigning to unwanted room
        return 100;
    } else if (!p && r) {
        // Cost of assigning empty room
        return 0;
    } else {
        // Should never occur
        throw std::runtime_error("Both people and rooms are padded!");
    }
    //
    return 0;
}