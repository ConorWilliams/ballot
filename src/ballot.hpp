// Copyright (C) 2020 Conor Williams

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include "structopt/app.hpp"
#include "structopt/sub_command.hpp"

/////////////////////////////////////////////////////////////////////////////

// Argument parsing

struct Args {
    struct Verify : structopt::sub_command {
        std::size_t index;
        std::string one_time_pad;                                     // Verifies this name
        std::optional<std::string> in_public = "public_ballot.json";  // Public ballot file
    };

    struct Run : structopt::sub_command {
        std::string in_people;

        std::optional<std::string> out_secret = "secret_ballot.csv";   // Write results here
        std::optional<std::string> out_public = "public_ballot.json";  // Write anonymised here
        std::optional<std::size_t> max_rooms;                          // Maximum num rooms to use
        std::optional<std::vector<std::string>> hostels;               // List of hostels
    };

    struct Cycle : structopt::sub_command {
        std::string in_people;
        std::vector<std::size_t> ks;
    };

    Args() = default;  // Required by structopt, cereal

    // Exceptions handled in constructor
    Args(int argc, char* argv[]) try : Args(structopt::app("Ballot").parse<Args>(argc, argv)) {
    } catch (structopt::exception& e) {
        std::cout << e.what() << "\n";
        std::cout << e.help();
        // Automatically re-throws
    }

    // Subcommands
    Verify verify;
    Run run;
    Cycle cycle;
};

STRUCTOPT(Args::Verify, index, one_time_pad);
STRUCTOPT(Args::Run, in_people, out_secret, out_public, max_rooms, hostels);
STRUCTOPT(Args::Cycle, in_people, ks);

STRUCTOPT(Args, run, verify, cycle);

/////////////////////////////////////////////////////////////////////////////

// Define People / Room types

namespace impl {

struct Person {
    std::string name{};

    std::string crsid{};

    std::size_t priority = 1;
    std::vector<std::string> pref{};
    std::string secret_name{};

    std::string one_time_pad{};
    std::size_t index{};

    [[nodiscard]] std::optional<std::size_t> choice_index(std::string const& r) const {
        for (std::size_t i = 0; i < pref.size(); i++) {
            if (pref[i] == r) {
                return i;
            }
        }
        return std::nullopt;
    }

    friend bool operator<(Person const& a, Person const& b) { return a.name < b.name; }

    template <class Archive> void serialize(Archive& archive) {
        archive(priority, pref, secret_name);
    }
};

}  // namespace impl

using Person = std::optional<impl::Person>;
using Room = std::optional<std::string>;

/////////////////////////////////////////////////////////////////////////////

std::vector<Person> parse_people(std::string const&);

std::vector<Room> find_rooms(std::vector<Person> const&);

void write_results(std::vector<std::pair<Person, Room>> const&, Args const&);

void highlight_results(std::vector<std::pair<Person, Room>> const&, Args const&);

template <typename F>
void analayse(std::vector<std::pair<Person, Room>> const& results, F&& is_hostel) {
    std::size_t count_normal = 0;
    std::size_t count_hostel = 0;
    std::size_t count_kicked = 0;

    std::size_t p_max = 0;

    for (auto&& [p, r] : results) {
        if (p) {
            p_max = std::max(p_max, p->priority);
        }
    }

    std::map<std::size_t, std::vector<std::size_t>> arr;

    std::vector<std::size_t> kicked(p_max + 1, 0);

    for (auto&& [p, r] : results) {
        if (p && r) {
            if (is_hostel(r)) {
                ++count_hostel;
            } else {
                ++count_normal;
            }

            if (std::optional i = p->choice_index(*r)) {
                auto [it, unwanted] = arr.try_emplace(*i, std::vector<std::size_t>(p_max + 1, 0));
                it->second[p->priority] += 1;
            } else {
                throw std::runtime_error("Person allocated to a room they didn't want!");
            }

        } else if (p && !r) {
            ++count_kicked;
            kicked[p->priority] += 1;
        }
    }

    std::cout << "-- Allocated " << count_normal + count_hostel;
    std::cout << " rooms of which " << count_hostel << " where hostels!\n";
    std::cout << "\n-- Summary:\n\n";

    std::cout << "   | Choice |";
    for (std::size_t i = 0; i < p_max + 1; i++) {
        if (i < 10) {
            std::cout << "  P" << i << " |";
        } else {
            std::cout << " P" << i << " |";
        }
    }
    std::cout << '\n';

    std::cout << "   |--------|";
    for (std::size_t i = 0; i < p_max + 1; i++) {
        std::cout << "-----|";
    }
    std::cout << '\n';

    for (auto&& [choice, v] : arr) {
        std::cout << "   |" << std::right << std::setw(7) << choice + 1 << " |";
        for (auto&& count : v) {
            std::cout << std::right << std::setw(4) << count << " |";
        }
        std::cout << '\n';
    }

    std::cout << "   |" << std::right << std::setw(9) << "Kicked |";
    for (auto&& count : kicked) {
        std::cout << std::right << std::setw(4) << count << " |";
    }
    std::cout << "\n\n";
}
