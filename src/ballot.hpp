#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "structopt/app.hpp"
#include "structopt/sub_command.hpp"

/////////////////////////////////////////////////////////////////////////////

// Helper class for exception handling upon CLI parsing
template <typename T> struct Parse : T {
    Parse(int argc, char* argv[]) try : T(structopt::app("ballot").parse<T>(argc, argv)) {
    } catch (structopt::exception& e) {
        std::cout << e.what() << "\n";
        std::cout << e.help();
    }
};

// This apps command line augment's struct
struct Args {
    struct Run : structopt::sub_command {
        std::string people;                                           // File containing people data
        std::optional<std::string> out_secret = "secret_ballot.csv";  // Write ballot results here
        std::optional<std::string> out_anon = "public_ballot.csv";    // Write anonymised input here
        std::optional<std::size_t> max_rooms;                         // Maximum num rooms to assign
    };

    struct Check : structopt::sub_command {
        std::string people;                    // File containing people data
        std::string secret_name;               // Name to check against
        std::optional<std::size_t> max_rooms;  // Maximum number of rooms to assign
    };

    // Sub-commands
    Check check;
    Run run;
};

STRUCTOPT(Args::Run, people, out_secret, out_anon, max_rooms);
STRUCTOPT(Args::Check, people, secret_name, max_rooms);
STRUCTOPT(Args, run, check);

/////////////////////////////////////////////////////////////////////////////

using RealRoom = std::string;

struct NullRoom {};

using Room = std::variant<NullRoom, RealRoom>;

struct NullPerson {};

struct AntiPerson {};

struct RealPerson {
    std::string name{};
    std::string crsid{};
    std::string secret_name{};

    std::vector<RealRoom> pref{};

    int priority = 1;
};

using Person = std::variant<NullPerson, RealPerson, AntiPerson>;

// Helper type for the std::visit
template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

/////////////////////////////////////////////////////////////////////////////

// Reads csv-file, expects header and columns: name, crsid, priority, choice 1, ..., choice n
std::vector<RealPerson> parse_people(Args const&);

// Find all the rooms the people have selected
std::vector<RealRoom> find_rooms(std::vector<RealPerson> const&);

void shuffle(std::vector<RealPerson>&);

void write_anonymised(std::vector<RealPerson> const&, Args const&);

void write_results(std::vector<std::optional<Person>> const&,
                   std::vector<std::optional<std::string>> const&,
                   Args const&);

void highlight_results(std::vector<std::optional<Person>> const&,
                       std::vector<std::optional<std::string>> const&,
                       Args const&);

// Convert vector of objects to vector of optional objects and pad with null-optional such that
// out.size() is greater than or equal to len
template <class T> std::vector<std::optional<T>> pad_null(std::vector<T>&& in, std::size_t len) {
    std::vector<std::optional<T>> out{std::move_iterator(in.begin()), std::move_iterator(in.end())};

    if (out.size() < len) {
        out.resize(len, std::nullopt);
    }

    return out;
}