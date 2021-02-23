
#include "ballot.hpp"

#include <bits/c++config.h>

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <fstream>
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
                 csv2::first_row_is_header<true>,
                 csv2::trim_policy::trim_characters<' ', '\r', '\n'>>
        csv;

    csv.mmap(args.people);  // Throws if no file

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
        // Check for odd characters, could suggest parsing problems
        for (auto&& h : p.pref) {
            for (auto&& c : h) {
                if (std::find(std::begin(charset), std::end(charset), c) == std::end(charset)) {
                    throw std::runtime_error("Unexpected characters in input file");
                }
            }
        }
    }

    return people;
}

// Find all the rooms people have selected
std::vector<RealRoom> find_rooms(std::vector<RealPerson> const& people) {
    // Using set (vs unordered_set) as it guarantees iteration order.
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

// Write out people with anonymised names to .csv that can be used to verify results
void write_anonymised(std::vector<RealPerson> const& people, Args const& args) {
    std::ofstream fstream(*args.out_anon);

    fstream << "name,crsid,priority";

    for (size_t i = 0; i < people[0].pref.size(); i++) {
        fstream << ",C" << i + 1;
    }

    for (auto&& p : people) {
        fstream << '\n' << p.secret_name << ",," << p.priority;
        for (auto&& room : p.pref) {
            fstream << ',' << room;
        }
    }
}

void write_results(std::vector<Person> const& people,
                   std::vector<Room> const& rooms,
                   Args const& args) {
    // Orders results for pretty printing
    std::set<std::string> ordered_results;

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

                stream << '\n' << p.name << ',' << p.crsid << ',' << p.priority;
                stream << ',' << "#" << k + 1 << ',' << r << ',' << p.secret_name;

                ordered_results.insert(stream.str());
            },
            [&](RealPerson const& p, Kicked const& r) {
                std::stringstream stream;

                stream << '\n' << p.name << ',' << p.crsid << ',' << p.priority;
                stream << ",," << r << ',' << p.secret_name;

                ordered_results.insert(stream.str());
            },
            [](auto&&...) {});
    }

    std::ofstream fstream(*args.out_secret);

    fstream << "name,crsid,priority,choice,room,secret_name";

    for (auto const& r : ordered_results) {
        fstream << r;
    }
}

void highlight_results(std::vector<Person> const& people,
                       std::vector<Room> const& rooms,
                       Args const& args) {
    for (std::size_t i = 0; i < people.size(); i++) {
        auto found = match(people[i], rooms[i])(
            [&](RealPerson const& p, auto const& r) {
                if (p.name == args.check_name) {
                    std::cout << "-- Your choices:";

                    for (auto&& room : p.pref) {
                        std::cout << ' ' << room;
                    }

                    std::cout << "\n-- You got room: " << r << '\n';

                    return true;
                } else {
                    return false;
                }
            },
            [](auto&&...) { return false; });

        if (found) {
            return;
        }
    }

    throw std::invalid_argument("Secret name not in list of people");
}
