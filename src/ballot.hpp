// Copyright (C) 2020 Conor Williams

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

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

        std::optional<std::vector<std::pair<std::string, int>>> test;
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
STRUCTOPT(Args::Run, in_people, out_secret, out_public, max_rooms, hostels, test);
STRUCTOPT(Args::Cycle, in_people, ks);
STRUCTOPT(Args, run, verify, cycle);

/////////////////////////////////////////////////////////////////////////////

// Define People / Room types

using RealRoom = std::string;

struct Kicked {};

using Room = std::variant<Kicked, RealRoom>;

struct NullPerson {};

struct AntiPerson {};

struct RealPerson {
    std::string name{};
    std::string crsid{};

    std::string secret_name{};
    std::vector<RealRoom> pref{};
    int priority = 1;

    template <class Archive> void serialize(Archive& archive) {
        archive(secret_name, pref, priority);
    }
};

using Person = std::variant<NullPerson, AntiPerson, RealPerson>;

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

void write_results(std::vector<Person> const&, std::vector<Room> const&, Args const&);

void highlight_results(std::vector<Person> const&, std::vector<Room> const&, Args const&);

template <typename U, typename T> std::vector<U> convert_vector(std::vector<T>&& tmp) {
    return {std::move_iterator(tmp.begin()), std::move_iterator(tmp.end())};
}
