#pragma once

#include <string>
#include <vector>

#include "csv.h"

struct Room {
    std::string name{};
    Room(std::string_view n) : name(n) {}
};

struct Person {
    std::string name{};
    std::vector<Room> preferences{};

    Person(std::string_view n) : name(n) {}
};
