// Copyright (C) 2020 Conor Williams

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "ballot.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/optional.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "cost.hpp"
#include "lapjv.hpp"

std::pair<std::vector<RealPerson>, std::vector<RealRoom>> load_data(Args& args) {
    std::vector<RealPerson> people;
    std::vector<RealRoom> rooms;

    if (args.check.has_value()) {
        std::ifstream file(*args.check.in_public);
        cereal::JSONInputArchive archive(file);
        archive(args.run.max_rooms, args.run.hostels, people, rooms);
    } else {
        people = parse_people(args);
        rooms = find_rooms(people);

        shuffle(people);  // Must randomise for fair ties break AND anonymity

        // Output for future checking
        std::ofstream file(*args.run.out_public);
        cereal::JSONOutputArchive archive(file);
        archive(args.run.max_rooms, args.run.hostels, people, rooms);
    }

    return {std::move(people), std::move(rooms)};
}

int main(int argc, char* argv[]) {
    Args args{argc, argv};

    auto [r_people, r_rooms] = load_data(args);

    std::cout << "-- There are " << r_people.size() << " people in the ballot.\n";
    std::cout << "-- Between them they selected " << r_rooms.size() << " rooms, ";

    auto is_hostel = [&](RealRoom const& room) -> bool {
        if (args.run.hostels) {
            for (auto&& prefix : *args.run.hostels) {
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

    // Convert to variants
    std::vector people = convert_vector<Person>(std::move(r_people));
    std::vector rooms = convert_vector<Room>(std::move(r_rooms));

    std::size_t num_people = people.size();

    // Need to add an anti-person for each room we wish to remove from the ballot
    if (args.run.max_rooms && rooms.size() > *args.run.max_rooms) {
        auto m = *args.run.max_rooms;
        std::cout << "-- You want to limit the number of rooms to " << m;
        std::cout << " so we are adding " << rooms.size() - m << " anti-people.\n";
        people.resize(people.size() + rooms.size() - m, AntiPerson{});
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

    if (args.run.has_value()) {
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
