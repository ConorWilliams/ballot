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
    std::string people_csv;                             // File containing people data
    std::optional<std::string> outfile = "ballot.csv";  // Write results here
    std::optional<bool> gen_secrets = false;            // Generate secrets so ballot can be checked
    std::optional<std::string> anon_csv = "anon.csv";   // Write anonymised results here
};

STRUCTOPT(Args, people_csv, outfile, gen_secrets, anon_csv);
