#include <iostream>
#include <string>

#include "structopt/app.hpp"

struct CommandLineArgs {
    std::string choice;
};

STRUCTOPT(CommandLineArgs, choice);

CommandLineArgs parse_clargs(int argc, char *argv[]) try {
    return structopt::app("olkmc").parse<CommandLineArgs>(argc, argv);
} catch (structopt::exception &e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
    throw e;
}

int main(int argc, char *argv[]) {
    CommandLineArgs clargs = parse_clargs(argc, argv);

    std::cout << "Working\n";

    return 0;
}
