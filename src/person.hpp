#pragma once

#include <csv2/reader.hpp>
#include <exception>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "clargs.hpp"

struct Person {
    std::string name{};

    int priority;

    std::vector<std::string> pref{};

  private:
    friend std::vector<Person> read_people(Args const& args);

    std::string secret_name;
};

std::vector<Person> read_people(Args const& args) {
    csv2::Reader csv;

    // std::random_device rd;
    // std::map<int, int> hist;
    // std::uniform_int_distribution<int> dist(0, 9);

    std::string buff;

    csv.mmap(args.people_csv);  // Throws if no file

    for (const auto row : csv) {
        Person p;
        int count = 0;
        for (const auto cell : row) {
            switch (count++) {
                case 0:
                    cell.read_value(p.name);
                    break;
                case 1:
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
    }

    return {};
}
