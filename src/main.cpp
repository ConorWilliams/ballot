// Copyright (C) 2020 Conor Williams

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <optional>
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

int main(int argc, char* argv[]) {
    // Automagically parses
    Args args{argc, argv};

    if (args.cycle.has_value()) {
        std::vector people = parse_people(args.cycle.in_people);

        for (auto&& k : args.cycle.ks) {
            report_k_cycles(k, people);
        }

        return 0;
    }

    std::vector<Person> people;
    std::vector<Room> rooms;

    if (args.verify.has_value()) {
        std::ifstream file(*args.verify.in_public);
        cereal::JSONInputArchive archive(file);
        archive(args.run.max_rooms, args.run.hostels, people, rooms);
    } else {
        people = parse_people(args.run.in_people);
        rooms = find_rooms(people);

        shuffle(people);  // Must randomise for fair ties break AND anonymity

        // Output for future verification
        std::ofstream file(*args.run.out_public);
        cereal::JSONOutputArchive archive(file);
        archive(args.run.max_rooms, args.run.hostels, people, rooms);
    }

    std::cout << "-- There are " << people.size() << " people in the ballot.\n";
    std::cout << "-- Between them they selected " << rooms.size() << " rooms, ";

    auto is_hostel = [&](Room const& room) -> bool {
        if (room && args.run.hostels) {
            for (auto&& prefix : *args.run.hostels) {
                if (room->starts_with(prefix)) {
                    return true;
                }
            }
        }
        return false;
    };

    std::size_t count = 0;

    for (auto&& room : rooms) {
        if (is_hostel(room)) {
            ++count;
        }
    }

    std::cout << "of which " << count << " are hostels.\n";

    std::vector<std::pair<Person, Room>> results{};

    // Must sort as we kick the (numerically highest) priority, use stable sort for
    // implementation inter-compatibility. All currently valid so can deference :)
    std::stable_sort(people.begin(), people.end(), [](Person const& a, Person const& b) {
        return a->priority < b->priority;
    });

    // Need to remove the people who missed the ballot
    if (args.run.max_rooms) {
        std::cout << "-- You want to limit the number of rooms to " << *args.run.max_rooms << '\n';
    }
    while (!people.empty() && args.run.max_rooms && people.size() > *args.run.max_rooms) {
        results.emplace_back(std::move(people.back()), std::nullopt);
        people.pop_back();
    }

    // For every real person we need the possibility of them being kicked off the ballot
    rooms.resize(people.size() + rooms.size(), std::nullopt);

    // Must pad people (always more than rooms) with null people for balanced assignment
    people.resize(rooms.size(), std::nullopt);

    linear_assignment(people, rooms, [&](Person const& p, Room const& r) {
        return cost_function(p, r, is_hostel);
    });

    // Build results
    for (std::size_t i = 0; i < people.size(); i++) {
        results.emplace_back(std::move(people[i]), std::move(rooms[i]));
    }

    // NOTE : people, rooms are All moved from

    if (args.run.has_value()) {
        write_results(results, args);
        analayse(results, is_hostel);
    } else {
        highlight_results(results, args);
    }

    return 0;
}
