#include <bits/c++config.h>

#include <iostream>
#include <string>
#include <vector>

#include "ballot.hpp"
#include "cli_args.hpp"
#include "wrap_lapjv.hpp"

int main(int argc, char* argv[]) {
    Parse<Args> args{argc, argv};

    auto people = parse_people(args);
    auto rooms = find_rooms(people);

    write_anonymised(people, args);

    std::size_t n = std::max(people.size(), rooms.size());

    auto pp = pad_null(std::move(people), n);
    auto rr = pad_null(std::move(rooms), n);

    for (auto&& elem : rr) {
        if (elem) {
            std::cout << *elem << '\n';
        } else {
            std::cout << "empty" << '\n';
        }
    }

    for (auto&& p : pp) {
        if (p) {
            std::cout << p->name << ' ' << p->crsid << ' ' << p->priority << ' ' << p->secret_name;
            for (auto&& h : p->pref) {
                std::cout << ' ' << h;
            }
            std::cout << '\n';
        } else {
            std::cout << "empty" << '\n';
        }
    }

    std::cout << "Cost=" << 1 << "\n";
    std::cout << "Working\n";

    return 0;
}
