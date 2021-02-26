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
#include <variant>
#include <vector>

#include "structopt/app.hpp"
#include "structopt/sub_command.hpp"

/////////////////////////////////////////////////////////////////////////////

// Argument parsing

struct Args {
    struct Verify : structopt::sub_command {
        std::string secret_name;                                      // Verifies this name
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

STRUCTOPT(Args::Verify, secret_name, in_public);
STRUCTOPT(Args::Run, in_people, out_secret, out_public, max_rooms, hostels);
STRUCTOPT(Args::Cycle, in_people, ks);

STRUCTOPT(Args, run, verify, cycle);

/////////////////////////////////////////////////////////////////////////////

// Define People / Room types

using RealRoom = std::string;

struct Kicked {};

using Room = std::variant<Kicked, RealRoom>;

struct NullPerson {};

struct RealPerson {
    std::string name{};
    std::string crsid{};

    std::string secret_name{};
    std::vector<RealRoom> pref{};
    std::size_t priority = 1;

    template <class Archive> void serialize(Archive& archive) {
        archive(secret_name, pref, priority);
    }
};

using Person = std::variant<NullPerson, RealPerson>;

/////////////////////////////////////////////////////////////////////////////

namespace impl {

template <class... Ts> struct overload : Ts... { using Ts::operator()...; };
template <class... Ts> overload(Ts...) -> overload<Ts...>;

}  // namespace impl

// Helper function for working with std::visit/variants
template <class... Vs> decltype(auto) match(Vs&&... vs) {
    return [... vs = std::forward<Vs>(vs)]<class... Lam>(Lam && ... lam) mutable->decltype(auto) {
        return std::visit(impl::overload{std::forward<Lam>(lam)...}, std::forward<Vs>(vs)...);
    };
}

/////////////////////////////////////////////////////////////////////////////

std::vector<RealPerson> parse_people(std::string const&);

std::vector<RealRoom> find_rooms(std::vector<RealPerson> const&);

void shuffle(std::vector<RealPerson>&);

void write_results(std::vector<std::pair<RealPerson, Room>> const&, Args const&);

void highlight_results(std::vector<std::pair<RealPerson, Room>> const&, Args const&);

template <typename U, typename T> std::vector<U> convert_vector(std::vector<T>&& tmp) {
    return {std::move_iterator(tmp.begin()), std::move_iterator(tmp.end())};
}

template <typename F>
inline void analayse(std::vector<std::pair<RealPerson, Room>> const& results, F&& is_hostel) {
    std::size_t count_normal = 0;
    std::size_t count_hostel = 0;
    std::size_t count_kicked = 0;

    std::size_t max_priority = 0;

    for (auto&& [p, r] : results) {
        max_priority = std::max(max_priority, p.priority);
    }

    std::map<std::size_t, std::vector<std::size_t>> arr;

    std::vector<std::size_t> kicked(max_priority + 1, 0);

    for (auto&& [person, room] : results) {
        match(room)(
            [&, p = person](RealRoom const& r) {
                if (is_hostel(r)) {
                    ++count_hostel;
                } else {
                    ++count_normal;
                }

                std::size_t const choice = [&] {
                    for (std::size_t i = 0; i < p.pref.size(); i++) {
                        if (p.pref[i] == r) {
                            return i;
                        }
                    }
                    throw std::runtime_error("Person allocated to a room they didn't want!");
                }();

                auto [it, inserted]
                    = arr.try_emplace(choice, std::vector<std::size_t>(max_priority + 1, 0));

                it->second[p.priority] += 1;
            },
            [&, p = person](Kicked const&) {
                ++count_kicked;
                kicked[p.priority] += 1;
            });
    }

    std::cout << "-- Allocated " << count_normal + count_hostel;
    std::cout << " rooms of which " << count_hostel << " where hostels!\n";

    std::cout << "\n-- Minima Breakdown:\n";

    for (auto&& [choice, v] : arr) {
        std::cout << "--    " << std::left << std::setw(4) << choice + 1;
        for (auto&& count : v) {
            std::cout << std::left << std::setw(4) << count;
        }
        std::cout << '\n';
    }

    std::cout << "--    " << std::left << std::setw(4) << "K";
    for (auto&& count : kicked) {
        std::cout << std::left << std::setw(4) << count;
    }
    std::cout << '\n';
}
