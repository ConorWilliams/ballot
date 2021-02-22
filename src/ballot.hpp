#pragma once

#include <string>
#include <vector>

#include "clargs.hpp"

struct Person {
    std::string name{};
    std::string crsid{};

    int priority;

    std::vector<std::string> pref{};

  private:
    friend std::vector<Person> parse_people(Args const& args);

    std::string secret_name{};
};

// Reads csv-file, expects header and columns: name, crsid, priority, choice 1, ..., choice n
std::vector<Person> parse_people(Args const& args);