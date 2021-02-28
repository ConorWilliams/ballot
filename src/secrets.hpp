// Copyright (C) 2020 Conor Williams

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "ballot.hpp"

std::string string_xor(std::string const&, std::string const&);

void anonymise_sort(std::vector<Person>&);
