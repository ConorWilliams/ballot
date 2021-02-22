#include <iostream>
#include <string>
#include <vector>

#include "clargs.hpp"
#include "person.hpp"
#include "wrap_lapjv.hpp"

int main(int argc, char *argv[]) {
    Parse<Args> clargs{argc, argv};

    read_people(clargs);

    std::cout << "Cost=" << 1 << "\n";
    std::cout << "Working\n";

    return 0;
}
