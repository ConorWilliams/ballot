#pragma once

#include <iterator>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "structopt/app.hpp"
#include "structopt/sub_command.hpp"

/////////////////////////////////////////////////////////////////////////////

struct Check : structopt::sub_command {
    std::string secret_name;                                      // Verifies results for this name
    std::optional<std::string> in_public = "public_ballot.json";  // Public ballot file
};

STRUCTOPT(Check, secret_name, in_public);

struct Run : structopt::sub_command {
    std::string in_people;

    std::optional<std::string> out_secret = "secret_ballot.csv";   // Write ballot results here
    std::optional<std::string> out_public = "public_ballot.json";  // Write anonymised input here
    std::optional<std::size_t> max_rooms;                          // Maximum number of rooms to use
    std::optional<std::vector<std::string>> hostels;               // List of hostels
};

STRUCTOPT(Run, in_people, out_secret, out_public, max_rooms, hostels);

struct Args {
    // Subcommands
    Check check;
    Run run;

    Args() = default;  // Required by structopt, cereal

    // Exceptions handled in constructor
    Args(int argc, char* argv[]) try : Args(structopt::app("Ballot").parse<Args>(argc, argv)) {
    } catch (structopt::exception& e) {
        std::cout << e.what() << "\n";
        std::cout << e.help();
        // Automatically re-throws
    }
};

STRUCTOPT(Args, run, check);

/////////////////////////////////////////////////////////////////////////////

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

// Reads csv-file, expects header and columns: name, crsid, priority, choice 1, ..., choice n
std::vector<RealPerson> parse_people(Args const&);

// Find all the rooms the people have selected
std::vector<RealRoom> find_rooms(std::vector<RealPerson> const&);

void shuffle(std::vector<RealPerson>&);

template <typename U, typename T> std::vector<U> convert_vector(std::vector<T>&& tmp) {
    return {std::move_iterator(tmp.begin()), std::move_iterator(tmp.end())};
}

void write_results(std::vector<Person> const&, std::vector<Room> const&, Args const&);

void highlight_results(std::vector<Person> const&, std::vector<Room> const&, Args const&);
