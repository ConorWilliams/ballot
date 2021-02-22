#include <iostream>
#include <string>
#include <vector>

#include "ballot.hpp"
#include "cli_args.hpp"
#include "cost.hpp"
#include "wrap_lapjv.hpp"

auto load_and_pad(Args const& args) {
    auto people = parse_people(args);
    auto rooms = find_rooms(people);

    if (args.run.has_value()) {
        shuffle(people);
        write_anonymised(people, args);
    }

    std::size_t n = std::max(people.size(), rooms.size());

    return std::pair{
        pad_null(std::move(people), n),
        pad_null(std::move(rooms), n),
    };
}

int main(int argc, char* argv[]) {
    Parse<Args> args{argc, argv};

    auto [people, rooms] = load_and_pad(args);

    double sum = linear_assignment(people, rooms, cost_function);

    write_results(people, rooms, args);

    std::cout << "Minimal cost = " << sum << std::endl;

    return 0;
}
