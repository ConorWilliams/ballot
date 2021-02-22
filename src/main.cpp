#include <iostream>
#include <string>
#include <vector>

#include "ballot.hpp"
#include "clargs.hpp"
#include "wrap_lapjv.hpp"

int main(int argc, char *argv[]) {
    Parse<Args> clargs{argc, argv};

    parse_people(clargs);

    std::cout << "Cost=" << 1 << "\n";
    std::cout << "Working\n";

    return 0;
}
