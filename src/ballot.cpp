
#include "ballot.hpp"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iterator>
#include <map>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "clargs.hpp"
#include "csv2/reader.hpp"
#include "picosha2.h"

namespace {

constexpr char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

std::random_device rng;

// Generate a random string of characters for salting
std::string random_string(std::size_t len = 32) {
    std::string out;
    std::sample(std::begin(charset), std::end(charset), std::back_inserter(out), len, rng);
    return out;
}

}  // namespace

// Reads csv-file, expects header and columns: name, crsid, priority, choice 1, ..., choice n
std::vector<Person> parse_people(Args const& args) {
    csv2::Reader csv;
    csv.mmap(args.people_csv);  // Throws if no file

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

        if (args.secrets) {
            picosha2::hash256_hex_string(p.name + random_string(), p.secret_name);
        }

        people.push_back(std::move(p));
    }

    return people;
}

// Find all the rooms the people have selected,
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
