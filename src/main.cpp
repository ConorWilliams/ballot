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

    shuffle(people);

    if (args.run.has_value()) {
        write_anonymised(people, args);
    }

    std::size_t n = std::max(people.size(), rooms.size());

    auto pp = pad_null(std::move(people), n);
    auto rr = pad_null(std::move(rooms), n);

    write_results(pp, rr, args);

    std::cout << "Working\n";

    return 0;
}
