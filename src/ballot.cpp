
#include "ballot.hpp"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iterator>
#include <optional>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

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
std::vector<Person> parse_people(Args const& args) {
    // These are all defaults, included for expressiveness
    csv2::Reader<csv2::delimiter<','>,
                 csv2::quote_character<'"'>,
                 csv2::first_row_is_header<true>,
                 csv2::trim_policy::trim_whitespace>
        csv;

    if (args.run.has_value()) {
        csv.mmap(args.run.people);  // Throws if no file
    } else {
        csv.mmap(args.check.people);
    }

    std::vector<Person> people;
    std::string buff;

    for (const auto row : csv) {
        Person p;
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

void shuffle(std::vector<Person>& people) { std::shuffle(people.begin(), people.end(), rng); }

// Write out people with anonymised names to .csv that can be used to verify results
void write_anonymised(std::vector<Person> const& people, Args const& args) {
    std::ofstream fstream(*args.run.out_anon);

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

void write_results(std::vector<std::optional<Person>> const& people,
                   std::vector<std::optional<std::string>> const& rooms,
                   Args const& args) {
    //
    if (args.run.has_value()) {
        std::ofstream fstream(*args.run.out_secret);

        fstream << "name,crsid,room,secret_name";

        for (std::size_t i = 0; i < people.size(); i++) {
            if (people[i]) {
                fstream << '\n' << people[i]->name << ',' << people[i]->crsid;

                if (rooms[i]) {
                    fstream << ',' << *rooms[i];
                } else {
                    fstream << ',' << "REJECTED";
                }

                fstream << ',' << people[i]->secret_name;
            }
        }
    } else {
        std::ofstream fstream(*args.check.out);

        fstream << "secret_name,room";

        for (std::size_t i = 0; i < people.size(); i++) {
            if (people[i]) {
                fstream << '\n' << people[i]->name;

                if (rooms[i]) {
                    fstream << ',' << *rooms[i];
                } else {
                    fstream << ',' << "REJECTED";
                }
            }
        }
    }
}

// Find all the rooms people have selected
std::vector<std::string> find_rooms(std::vector<Person> const& people) {
    // Using set (vs unordered_set) as it guarantees iteration order.
    std::set<std::string_view> rooms;

    for (auto const& person : people) {
        for (std::string_view room : person.pref) {
            if (!rooms.count(room)) {
                rooms.insert(room);
            }
        }
    }

    return {rooms.begin(), rooms.end()};
}
