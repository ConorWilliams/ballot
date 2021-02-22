#pragma once

#include <optional>
#include <string>
#include <vector>

#include "cli_args.hpp"

struct Person {
    std::string name{};
    std::string crsid{};
    std::string secret_name{};

    int priority;

    std::vector<std::string> pref{};

  private:
    friend std::vector<Person> parse_people(Args const& args);
};

// Reads csv-file, expects header and columns: name, crsid, priority, choice 1, ..., choice n
std::vector<Person> parse_people(Args const&);

void write_anonymised(std::vector<Person> const&, Args const&);

// Find all the rooms the people have selected
std::vector<std::string> find_rooms(std::vector<Person> const&);

// Convert vector of objects to vector of optional objects and pad with nullopt such that lengths is
// greater than or equal to len
template <class T> std::vector<std::optional<T>> pad_null(std::vector<T>&& in, std::size_t len) {
    std::vector<std::optional<T>> out{std::move_iterator(in.begin()), std::move_iterator(in.end())};

    if (out.size() < len) {
        out.resize(len, std::nullopt);
    }

    return out;
}