#pragma once

#include <optional>
#include <string>

#include "ballot.hpp"

// Cost function - overall cost is minimised
inline auto cost_function = [](std::optional<Person> const&, std::optional<std::string> const&) {
    //
    return 0;
};