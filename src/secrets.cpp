// Copyright (C) 2020 Conor Williams

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "secrets.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <random>
#include <stdexcept>
#include <string>

#include "ballot.hpp"
#include "picosha2.h"

namespace {  // Like static

constexpr char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

std::random_device true_rng{};

// Generate a random string of characters for salting names before hash
std::string random_string(std::size_t len) {
    std::string out;
    for (std::size_t i = 0; i < len; i++) {
        out.push_back(charset[true_rng() % (sizeof(charset) - 1)]);
    }

    return out;
}

}  // namespace

std::string string_xor(std::string const& a, std::string const& b) {
    assert(a.size() == b.size());

    std::string out{};

    for (std::size_t i = 0; i < a.size(); i++) {
        out.push_back(a[i] ^ b[i]);
    }

    return out;
}

// Here we want to deterministically "randomise" the order of the people and encrypt their names
void anonymise_sort(std::vector<Person>& people) {
    // Find longest name
    std::size_t w = [&] {
        std::size_t w = 0;

        for (auto&& p : people) {
            if (p) {
                w = std::max(p->name.size(), w);
            }
        }

        return w;
    }();

    constexpr size_t len = 32;

    if (w > len) {
        throw std::runtime_error("Name too long");
    }

    // Must pad all names to len to avoid leaking information
    for (auto&& p : people) {
        if (p) {
            p->name.resize(len, ' ');
            p->one_time_pad = random_string(len);
            p->secret_name = string_xor(p->name, p->one_time_pad);
        }
    }

    // Sort so results cannot be determined by input sequence ordering
    std::stable_sort(people.begin(), people.end());

    // Build seed deterministically
    std::string entropy;

    for (auto&& p : people) {
        if (p) {
            entropy.append(p->name);
        }
    }

    // Hash for security
    std::vector<unsigned char> hash(picosha2::k_digest_size);
    picosha2::hash256(entropy.begin(), entropy.end(), hash.begin(), hash.end());

    // Seed random number generator
    std::seed_seq s(hash.begin(), hash.end());
    std::mt19937 gen;
    gen.seed(s);

    // Now in almost-random order
    std::shuffle(people.begin(), people.end(), gen);
}
