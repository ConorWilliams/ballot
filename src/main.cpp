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
#include "collusion.hpp"
#include "cost.hpp"
#include "lapjv.hpp"

std::pair<std::vector<RealPerson>, std::vector<RealRoom>> load_data(Args& args) {
    std::vector<RealPerson> people;
    std::vector<RealRoom> rooms;

    if (args.verify.has_value()) {
        std::ifstream file(*args.verify.in_public);
        cereal::JSONInputArchive archive(file);
        archive(args.run.max_rooms, args.run.hostels, people, rooms);
    } else {
        people = parse_people(args.run.in_people);
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
    // Automagically parses
    Args args{argc, argv};

    if (args.cycle.has_value()) {
        std::vector<RealPerson> people = parse_people(args.cycle.in_people);

        for (auto&& k : args.cycle.ks) {
            report_k_cycles(k, people);
        }

        return 0;
    }

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

    // Must sort as we kick the (numerically highest) priority, must use stable for
    // implementation inter-compatibility
    std::stable_sort(
        r_people.begin(), r_people.end(), [](RealPerson const& a, RealPerson const& b) {
            return a.priority < b.priority;
        });

    // Convert to variants
    std::vector people = convert_vector<Person>(std::move(r_people));
    std::vector rooms = convert_vector<Room>(std::move(r_rooms));

    std::vector<std::pair<Person, Room>> results{};

    // Need to remove the people who missed the ballot
    if (args.run.max_rooms) {
        std::cout << "-- You want to limit the number of rooms to " << *args.run.max_rooms << '\n';
    }
    while (args.run.max_rooms && people.size() > *args.run.max_rooms) {
        results.emplace_back(people.back(), Kicked{});
        people.pop_back();
    }

    // For every real person we need the possibility of them being kicked off the ballot
    rooms.resize(people.size() + rooms.size(), Kicked{});

    // Must pad people (always more than rooms) with null people for balanced assignment
    people.resize(rooms.size(), NullPerson{});

    linear_assignment(people, rooms, [&](Person const& p, Room const& r) {
        return cost_function(p, r, is_hostel);
    });

    if (args.run.has_value()) {
        write_results(people, rooms, args);
        analayse(people, rooms, is_hostel);
    } else {
        highlight_results(people, rooms, args);
    }

    return 0;
}
