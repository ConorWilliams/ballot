// Copyright (C) 2020 Conor Williams

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "ballot.hpp"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "csv2/parameters.hpp"
#include "csv2/reader.hpp"
#include "picosha2.h"

namespace {  // Like static

constexpr char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

std::random_device true_rng;

// Generate a random string of characters for salting names before hash
std::string random_string(std::size_t len = 32) {
    std::string out;
    std::sample(std::begin(charset), std::end(charset), std::back_inserter(out), len, true_rng);
    return out;
}

}  // namespace

// Reads csv-file, expects header and columns: name, crsid, priority, choice 1, ..., choice n
std::vector<Person> parse_people(std::string const& fname) {
    // Have to manually include carriage return ('\r')
    csv2::Reader<csv2::delimiter<','>,
                 csv2::quote_character<'"'>,
                 csv2::first_row_is_header<false>,
                 csv2::trim_policy::trim_characters<' ', '\r', '\n'>>
        csv;

    csv.mmap(fname);  // Throws if no file

    std::vector<Person> people;

    for (std::string buff; const auto row : csv) {
        impl::Person real_p;
        std::size_t count = 0;
        for (const auto cell : row) {
            switch (count++) {
                case 0:
                    cell.read_value(real_p.name);
                    break;
                case 1:
                    cell.read_value(real_p.crsid);
                    break;
                case 2:
                    buff.clear();
                    cell.read_value(buff);
                    real_p.priority = std::stoi(buff);
                    break;
                default:
                    buff.clear();
                    cell.read_value(buff);
                    real_p.pref.push_back(buff);
            }
        }

        picosha2::hash256_hex_string(real_p.name + random_string(), real_p.secret_name);

        people.push_back(std::move(real_p));
    }

    // verify all people have made the same number of choices and that there is at least one person

    if (people.empty()) {
        throw std::runtime_error("No people in csv");
    }

    std::size_t const k = people[0]->pref.size();

    for (auto const& p : people) {
        if (p->pref.size() != k) {
            throw std::runtime_error(
                "Not all people have made the same number of choices, maybe a trailing newline");
        }
    }

    return people;
}

// Find all the rooms people have selected
std::vector<Room> find_rooms(std::vector<Person> const& people) {
    // Using set (vs unordered_set) as it guarantees iteration order, otherwise results platform
    // dependant in-case of degenerate minima.
    std::set<std::string_view> rooms;

    for (auto const& person : people) {
        if (person) {
            for (std::string_view room : person->pref) {
                if (!rooms.count(room)) {
                    rooms.insert(room);
                }
            }
        }
    }

    return {rooms.begin(), rooms.end()};
}

void shuffle(std::vector<Person>& people) { std::shuffle(people.begin(), people.end(), true_rng); }

void write_results(std::vector<std::pair<Person, Room>> const& result, Args const& args) {
    // Orders results for pretty printing
    std::set<std::pair<std::string, std::string>> ordered_results;

    auto sorted = result;

    std::sort(sorted.begin(), sorted.end());

    // Find longest name
    std::size_t w = [&] {
        std::size_t w = 0;

        for (auto&& [person, room] : sorted) {
            if (person) {
                w = std::max(person->name.size(), w);
            }
        }

        return w;
    }();

    for (std::ofstream fstream{*args.run.out_secret}; auto&& [person, room] : sorted) {
        if (person) {
            fstream << std::left << std::setw(w + 2) << person->name;

            fstream << std::left << std::setw(18) << "," + person->crsid;
            fstream << std::left << std::setw(4) << ",P" + std::to_string(person->priority);

            if (room) {
                if (std::optional i = person->choice_index(*room)) {
                    fstream << std::left << std::setw(5) << ",#" + std::to_string(*i + 1);
                    fstream << std::left << std::setw(6) << "," + *room;
                } else {
                    throw std::runtime_error("Person allocated to a room they didn't want!");
                }
            } else {
                fstream << std::left << std::setw(5) << ",#";
                fstream << std::left << std::setw(6) << ",KICK";
            }

            fstream << ',' << person->secret_name << '\n';
        }
    }
}

void highlight_results(std::vector<std::pair<Person, Room>> const& results, Args const& args) {
    for (auto&& [person, room] : results) {
        if (person && person->secret_name == args.verify.secret_name) {
            std::cout << "-- Your choices :";

            for (std::string_view room : person->pref) {
                std::cout << ' ' << room;
            }

            std::cout << "\n-- Your priority: " << person->priority;

            if (room) {
                std::cout << "\n-- You got room : " << *room << '\n';
            } else {
                std::cout << "\n-- You got KICKED\n";
            }

            return;
        }
    }
    throw std::invalid_argument("secret_name: \"" + args.verify.secret_name + "\" not in "
                                + *args.verify.in_public);
}
