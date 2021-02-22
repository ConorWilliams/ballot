
#include "person.hpp"

#include <cstdlib>
#include <exception>
#include <iterator>
#include <random>
#include <string>
#include <vector>

#include "clargs.hpp"
#include "csv2/reader.hpp"
#include "picosha2.h"

namespace {

constexpr char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

std::random_device rng{};
std::uniform_int_distribution<> dist{0, sizeof(charset) - 1};

// Generate a random string of characters for salting
std::string random_string(std::size_t len = 32) {
    auto randchar = [&]() -> char { return charset[dist(rng)]; };
    std::string out;
    std::generate_n(std::back_inserter(out), len, randchar);
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

    for (auto&& p : people) {
        std::cout << p.name << ' ' << p.crsid << ' ' << p.priority << ' ' << p.secret_name;
        for (auto&& h : p.pref)

        {
            std::cout << ' ' << h;
        }
        std::cout << '\n';
    }

    return people;
}
