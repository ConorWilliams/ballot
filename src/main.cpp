

#include <bits/c++config.h>

#include <iostream>
#include <string>
#include <vector>

#include "ballot.hpp"
#include "cost.hpp"
#include "lapjv.hpp"

int main(int argc, char* argv[]) {
    Args args{argc, argv};

    std::vector<RealPerson> r_people = parse_people(args);
    std::vector<RealRoom> r_rooms = find_rooms(r_people);

    if (!args.check_name) {
        shuffle(r_people);                 // Must randomise to break ties fairly
        write_anonymised(r_people, args);  // Can be re-consumed in check mode
    }

    // Convert to variants
    std::vector people = convert_vector<Person>(std::move(r_people));
    std::vector rooms = convert_vector<Room>(std::move(r_rooms));

    std::size_t num_people = people.size();

    // Need to add an anti-person for each room we wish to remove from the ballot
    if (args.max_rooms && rooms.size() > *args.max_rooms) {
        people.resize(people.size() + rooms.size() - *args.max_rooms, AntiPerson{});
    }

    // For every real person we need the possibility of them being kicked off the ballot
    rooms.resize(rooms.size() + num_people, Kicked{});

    // Must pad people (always more than rooms) with null people for balanced assignment
    assert(people.size() <= rooms.size());
    people.resize(rooms.size(), NullPerson{});

    double sum = linear_assignment(people, rooms, &cost_function);

    if (!args.check_name) {
        // TODO : sort alphabetically
        write_results(people, rooms, args);
        std::cout << "Minimal cost = " << sum << std::endl;
    } else {
        // highlight_results(people, rooms, args);
    }

    return 0;
}
