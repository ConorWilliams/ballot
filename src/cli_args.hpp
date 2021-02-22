#pragma once

#include <exception>
#include <string>

#include "structopt/app.hpp"
#include "structopt/sub_command.hpp"

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
    struct Run : structopt::sub_command {
        std::string people;                                           // File with people data
        std::optional<std::string> out_secret = "secret_ballot.csv";  // Write results here
        std::optional<std::string> out_anon = "anon_people.csv";  // Write anonymised results here
    };

    struct Check : structopt::sub_command {
        std::string people;                             // File containing people data
        std::optional<std::string> out = "ballot.csv";  // Write results here
    };

    // Sub-commands
    Run run;
    Check check;

    // Symmetric access
    std::string const &people() const {
        if (run.has_value()) {
            return run.people;
        } else {
            return check.people;
        }
    }
};

STRUCTOPT(Args::Run, people, out_secret, out_anon);
STRUCTOPT(Args::Check, people, out);
STRUCTOPT(Args, run, check);
