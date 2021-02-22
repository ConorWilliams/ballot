#pragma once

#include <exception>
#include <string>

#include "structopt/app.hpp"

// Helper class for exception handling upon CLI parsing
template <typename T> struct Parse : T {
    Parse(int argc, char *argv[]) try : T(structopt::app("ballot").parse<T>(argc, argv)) {
    } catch (structopt::exception &e) {
        std::cout << e.what() << "\n";
        std::cout << e.help();
    }
};

// This apps command line augment's struct
struct Args {
    std::string people_csv;

    std::optional<bool> secrets = false;
};

STRUCTOPT(Args, people_csv, secrets);
