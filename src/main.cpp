#include <iostream>
#include <string>
#include <vector>

#include "clargs.hpp"
#include "wrap_lapjv.hpp"

struct Person {
    std::string name;

    Person(std::string_view name) : name(name) {}
};

struct Room {
    std::string name;

    Room(std::string_view name) : name(name) {}
};

int main(int argc, char *argv[]) {
    Parse<Args> clargs{argc, argv};

    std::vector<Person> people{Person("CJ"), Person("Rebecca")};

    std::vector<Room> rooms{Room("10"), Room("9")};

    double cost = linear_assignment(people, rooms, [](Person const &p, Room const &r) {
        //
        if (p.name == "CJ") {
            if (r.name == "9") {
                return 5;
            } else {
                return 1;
            }
        }
        if (p.name == "Rebecca") {
            if (r.name == "10") {
                return 10;
            } else {
                return 0;
            }
        }
        return 1;
    });

    for (size_t i = 0; i < people.size(); i++) {
        std::cout << people[i].name << " to " << rooms[i].name << '\n';
    }

    std::cout << "Cost=" << cost << "\n";
    std::cout << "Working\n";

    return 0;
}
