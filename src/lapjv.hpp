// Copyright (C) 2020 Conor Williams

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "lap.h"

/*
 *  Provide type && memory safe interface to LAPJV linear assignment optimiser.
 *  Reorders "tasks" such that agent[i] is assigned to task[i].
 *  Finds the global (but possibly degenerate) minima of:
 *
 *      sum_i f(agent[i], task[i])
 *
 *  with f the supplied cost function.
 */
template <class Agent, class Task, class Cost>
std::enable_if_t<std::is_invocable_r_v<double, Cost, Agent const &, Task const &>, double>
linear_assignment(std::vector<Agent> const &agents, std::vector<Task> &tasks, Cost &&f) {
    // Balanced assignment
    if (agents.size() != tasks.size()) {
        throw std::invalid_argument("Requires same number of agents and task");
    }

    // Decision to use c-style pointers as wrapping cost** with unique pointers is non-trivial

    int dim = agents.size();

    // Note that col, row, cost these types are typedef-ed in lap.h

    // Allocate array
    cost **cost_matrix = [&] {
        cost **tmp = new cost *[dim];
        for (int i = 0; i < dim; i++) {
            tmp[i] = new cost[dim];
        }
        return tmp;
    }();

    col *rowsol = new col[dim];
    row *colsol = new row[dim];
    cost *u = new cost[dim];
    cost *v = new cost[dim];

    // Assign costs to the cost_matrix
    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            cost_matrix[i][j] = std::invoke(f, agents[i], tasks[j]);
        }
    }

    // Use lap algorithm to calculate the minimum total cost
    cost cost_sum = lap(dim, cost_matrix, rowsol, colsol, u, v);

    {
        // Reorder tasks
        std::vector<Task> ordered_tasks;

        for (int i = 0; i < dim; i++) {
            ordered_tasks.push_back(std::move(tasks[rowsol[i]]));
        }

        using std::swap;
        swap(ordered_tasks, tasks);
    }

    // Release memory
    for (int i = 0; i < dim; ++i) {
        delete[] cost_matrix[i];
    }
    delete[] cost_matrix;

    delete[] rowsol;
    delete[] colsol;
    delete[] u;
    delete[] v;

    return cost_sum;
}
