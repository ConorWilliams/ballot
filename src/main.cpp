#include <algorithm>
#include <cassert>
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

    std::cout << "-- There are " << r_people.size() << " people in the ballot.\n";
    std::cout << "-- Between them they selected " << r_rooms.size() << " rooms, ";

    auto is_hostel = [&](RealRoom const& room) -> bool {
        if (args.hostels) {
            for (auto&& prefix : *args.hostels) {
                if (room.starts_with(prefix)) {
                    return true;
                }
            }
        }
        return false;
    };

    std::size_t count = 0;
    for (auto&& room : r_rooms) {
        if (is_hostel(room)) {
            ++count;
        }
    }

    std::cout << "of which " << count << " are hostels.\n";

    if (!args.check_name) {
        shuffle(r_people);                 // Must randomise for fair ties break AND anonymity
        write_anonymised(r_people, args);  // Can be re-consumed in check mode
    }

    // Convert to variants
    std::vector people = convert_vector<Person>(std::move(r_people));
    std::vector rooms = convert_vector<Room>(std::move(r_rooms));

    std::size_t num_people = people.size();

    // Need to add an anti-person for each room we wish to remove from the ballot
    if (args.max_rooms && rooms.size() > *args.max_rooms) {
        std::cout << "-- You want to limit the number of rooms to " << *args.max_rooms;
        std::cout << " so we are adding " << rooms.size() - *args.max_rooms << " anti-people.\n";
        people.resize(people.size() + rooms.size() - *args.max_rooms, AntiPerson{});
    }

    // For every real person we need the possibility of them being kicked off the ballot
    rooms.resize(rooms.size() + num_people, Kicked{});

    // Must pad people (always more than rooms) with null people for balanced assignment
    assert(people.size() <= rooms.size());
    people.resize(rooms.size(), NullPerson{});

    std::cout << "-- Running minimisation...\n";
    double sum = linear_assignment(people, rooms, [&](Person const& p, Room const& r) {
        return cost_function(p, r, is_hostel);
    });

    if (!args.check_name) {
        write_results(people, rooms, args);

        std::size_t count_normal = 0;
        std::size_t count_hostel = 0;
        std::size_t count_kicked = 0;

        for (std::size_t i = 0; i < people.size(); i++) {
            match(people[i], rooms[i])(
                [&](RealPerson&, RealRoom& r) {
                    if (is_hostel(r)) {
                        ++count_hostel;
                    } else {
                        ++count_normal;
                    }
                },
                [&](RealPerson&, Kicked&) { ++count_kicked; },
                [&](auto&...) {});
        }

        std::cout << "-- Minima Breakdown:\n";
        std::cout << "--    Normal: " << count_normal << '\n';
        std::cout << "--    Hostel: " << count_hostel << '\n';
        std::cout << "--    Kicked: " << count_kicked << '\n';

        std::cout << "-- The final cost was " << sum << std::endl;
    } else {
        highlight_results(people, rooms, args);
    }

    return 0;
}
