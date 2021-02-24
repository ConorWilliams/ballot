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

std::random_device rng;

// Generate a random string of characters for salting names before hash
std::string random_string(std::size_t len = 32) {
    std::string out;
    std::sample(std::begin(charset), std::end(charset), std::back_inserter(out), len, rng);
    return out;
}

}  // namespace

// Reads csv-file, expects header and columns: name, crsid, priority, choice 1, ..., choice n
std::vector<RealPerson> parse_people(Args const& args) {
    // Have to manually include carriage return ('\r')
    csv2::Reader<csv2::delimiter<','>,
                 csv2::quote_character<'"'>,
                 csv2::first_row_is_header<false>,
                 csv2::trim_policy::trim_characters<' ', '\r', '\n'>>
        csv;

    csv.mmap(args.run.in_people);  // Throws if no file

    std::vector<RealPerson> people;

    for (std::string buff; const auto row : csv) {
        RealPerson p;
        int count = 0;
        for (const auto cell : row) {
            switch (count++) {
                case 0:
                    cell.read_value(p.name);
                    break;
                case 1:
                    cell.read_value(p.crsid);
                    break;
                case 2:
                    buff.clear();
                    cell.read_value(buff);
                    p.priority = std::stoi(buff);
                    break;
                default:
                    buff.clear();
                    cell.read_value(buff);
                    p.pref.push_back(buff);
            }
        }

        picosha2::hash256_hex_string(p.name + random_string(), p.secret_name);

        people.push_back(std::move(p));
    }

    // Check all people have made the same number of choices and that there is at least one person

    if (people.empty()) {
        throw std::runtime_error("No people in csv");
    }

    std::size_t const k = people[0].pref.size();

    for (auto const& p : people) {
        if (p.pref.size() != k) {
            throw std::runtime_error(
                "Not all people have made the same number of choices, maybe a trailing newline");
        }
    }

    return people;
}

// Find all the rooms people have selected
std::vector<RealRoom> find_rooms(std::vector<RealPerson> const& people) {
    // Using set (vs unordered_set) as it guarantees iteration order, otherwise results platform
    // dependant in-case of degenerate minima.
    std::set<RealRoom> rooms;

    for (auto const& person : people) {
        for (auto const& room : person.pref) {
            if (!rooms.count(room)) {
                rooms.insert(room);
            }
        }
    }

    return {rooms.begin(), rooms.end()};
}

void shuffle(std::vector<RealPerson>& people) { std::shuffle(people.begin(), people.end(), rng); }

void write_results(std::vector<Person> const& people,
                   std::vector<Room> const& rooms,
                   Args const& args) {
    // Orders results for pretty printing
    std::set<std::pair<std::string, std::string>> ordered_results;

    for (std::size_t i = 0; i < people.size(); i++) {
        match(people[i], rooms[i])(
            [&](RealPerson const& p, RealRoom const& r) {
                // Get choice index
                std::size_t const k = [&] {
                    for (std::size_t i = 0; i < p.pref.size(); i++) {
                        if (p.pref[i] == r) {
                            return i;
                        }
                    }
                    throw std::runtime_error("Person allocated to a room they didn't want!");
                }();

                std::stringstream stream;

                stream << std::left << std::setw(18) << "," + p.crsid;
                stream << std::left << std::setw(4) << ",P" + std::to_string(p.priority);
                stream << std::left << std::setw(5) << ",#" + std::to_string(k + 1);
                stream << std::left << std::setw(6) << "," + r;
                stream << ',' << p.secret_name;

                ordered_results.emplace(p.name, stream.str());
            },
            [&](RealPerson const& p, Kicked const&) {
                std::stringstream stream;

                stream << std::left << std::setw(18) << "," + p.crsid;
                stream << std::left << std::setw(4) << ",P" + std::to_string(p.priority);
                stream << std::left << std::setw(5) << ",#";
                stream << std::left << std::setw(6) << ",KICK";
                stream << ',' << p.secret_name;

                ordered_results.emplace(p.name, stream.str());
            },
            [](auto&&...) {});
    }

    // Find longest name
    std::size_t w = [&] {
        std::size_t w = 0;

        for (auto&& [name, str] : ordered_results) {
            w = std::max(name.size(), w);
        }

        return w;
    }();

    for (std::ofstream fstream{*args.run.out_secret}; auto&& [name, str] : ordered_results) {
        fstream << std::left << std::setw(w + 2) << name;
        fstream << str << '\n';
    }
}

void highlight_results(std::vector<Person> const& people,
                       std::vector<Room> const& rooms,
                       Args const& args) {
    for (std::size_t i = 0; i < people.size(); i++) {
        auto found = match(people[i])(
            [&](RealPerson const& p) {
                if (p.secret_name == args.check.secret_name) {
                    std::cout << "-- Your choices:";

                    for (auto&& room : p.pref) {
                        std::cout << ' ' << room;
                    }

                    match(rooms[i])(
                        [](RealRoom const& r) { std::cout << "\n-- You got room: " << r << '\n'; },
                        [](Kicked const&) { std::cout << "\n-- You got KICKED\n"; });

                    return true;
                } else {
                    return false;
                }
            },
            [](auto) { return false; });

        if (found) {
            return;
        }
    }

    throw std::invalid_argument("secret_name: \"" + args.check.secret_name + "\" not in "
                                + *args.check.in_public);
}
