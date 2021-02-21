#include <iostream>

#include "clargs.hpp"

int main(int argc, char *argv[]) {
    Parse<Args> clargs{argc, argv};

    std::cout << "Working\n";

    return 0;
}
