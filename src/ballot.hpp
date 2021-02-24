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
    struct Check : structopt::sub_command {
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

    Args() = default;  // Required by structopt, cereal

    // Exceptions handled in constructor
    Args(int argc, char* argv[]) try : Args(structopt::app("Ballot").parse<Args>(argc, argv)) {
    } catch (structopt::exception& e) {
        std::cout << e.what() << "\n";
        std::cout << e.help();
        // Automatically re-throws
    }

    // Subcommands
    Check check;
    Run run;
};

STRUCTOPT(Args::Check, secret_name, in_public);
STRUCTOPT(Args::Run, in_people, out_secret, out_public, max_rooms, hostels);
STRUCTOPT(Args, run, check);

/////////////////////////////////////////////////////////////////////////////

// Define People / Room types

using RealRoom = std::string;

struct Kicked {
    friend std::ostream& operator<<(std::ostream& os, Kicked const&) { return os << "KICKED"; }
};

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

std::vector<RealPerson> parse_people(Args const&);

std::vector<RealRoom> find_rooms(std::vector<RealPerson> const&);

void shuffle(std::vector<RealPerson>&);

void write_results(std::vector<Person> const&, std::vector<Room> const&, Args const&);

void highlight_results(std::vector<Person> const&, std::vector<Room> const&, Args const&);

template <typename U, typename T> std::vector<U> convert_vector(std::vector<T>&& tmp) {
    return {std::move_iterator(tmp.begin()), std::move_iterator(tmp.end())};
}
