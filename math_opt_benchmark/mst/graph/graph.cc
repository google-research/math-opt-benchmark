// Copyright 2020 The MathOpt Benchmark Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "graph.h"
#include "flow.h"
#include "absl/strings/str_join.h"

#include <cmath>

#include "absl/random/random.h"

namespace math_opt_benchmark {

void debug_graph(const std::vector<std::vector<int>>& edges, int n) {
  std::cout << "[D] " << n << std::endl;
  for (auto& vector : edges) {
    std::cout << "[D] Graph: " << absl::StrJoin(vector, ",") << std::endl;
  }
}

/**
 * Copy preprocessed edges into the graph
 * @param edges Temporary list of adjacent vertices for each vertex passed by
 * std::move()
 */
Graph::Graph(std::vector<std::vector<int>>&& edges) {
  n_ = edges.size();
  edges_ = std::move(edges);
}

/**
 * Find cuts with no crossing edges
 * @param solution Solution values from LP solve routine
 * @return Vector of cuts
 */
std::vector<std::vector<int>> Graph::invalid_components(
    const Matrix<double>& x_values) {
  std::vector<bool> visited(n_, false);
  int num_visited = 0;
  std::vector<std::vector<int>> components;
  std::vector<int> stack;
  while (num_visited < n_) {
    components.emplace_back(0);
    std::vector<int> component;
    int index;
    for (index = 0; index < n_ && visited[index]; ++index);
    CHECK_LT(index, n_);
    stack.push_back(index);
    visited[index] = true;
    while (!stack.empty()) {
      int head = stack.back();
      stack.pop_back();
      component.push_back(head);
      for (const int& other : edges_[head]) {
        if (!visited[other]) {
          stack.push_back(other);
          visited[other] = true;
        }
      }
    }
    num_visited += component.size();
    components.push_back(component);
  }

  // First component seems to always be empty but that doesn't matter
  std::vector<std::vector<int>> invalid;
  for (const std::vector<int>& component : components) {
    // Check if connected component violates constraint (i.e. sum of edges > |component|-1)
    double sum = 0;
    for (const int& v1 : component) {
      for (const int& v2 : edges_[v1]) {
        sum += x_values.get(v1, v2);
      }
    }
    // Edges are counted twice in the graph
    // Component.size - 1 -> inf when component is empty (underflow)
    if (sum / 2 > component.size() - 1 + kTolerance) {
      invalid.push_back(component);
    }
  }
  return invalid;
}


std::vector<int> Graph::separation_oracle(const Matrix<double> &x_values) {
  int s = n_;
  int t = n_ + 1;
  std::vector<int> solution;
  for (int i = 0; i < n_; i++) {
    for (int j = i+1; j < n_; j++) {
      FlowSolver flow;
      for (int v1 = 0; v1 < edges_.size(); v1++) {
        double sum = 0;
        for (const int &v2 : edges_[v1]) {
          double capacity = x_values.get(v1, v2) / 2;
          flow.add(v1, v2, capacity);
          sum += capacity;

        }
        double capacity = (v1 == i || v1 == j) ? kMaxCap : sum;
        flow.add(s, v1, capacity);
        flow.add(v1, t, 1);
        flow.assertOpt();
        if (flow.generate_solution(solution)) {
          return solution;
        }
      }
    }
  }

  return solution;
}

// Assumes there is a path from v1 to v2
std::vector<int> Graph::find_path(int v1, int v2, std::vector<std::vector<int>> incidence) {
  std::vector<int> stack = {v1};
  std::vector<int> depths = {0};
  std::vector<int> path = {};
  int v = v1;
  int depth = 0;
  while (v != v2) {
    v = stack.back();
    depth = depths.back();
    stack.pop_back();
    depths.pop_back();
    for (int i = path.size() - 1; i >= depth; i--) {
      path.pop_back();
    }
    path.push_back(v);
    for (int j = 0; j < incidence.size(); j++) {
      if (incidence[v][j]) {
        if (j == v2) {
          v = v2;
          break;
        }
        stack.push_back(j);
        depths.push_back(depth + 1);
        incidence[v][j] = incidence[j][v] = 0;
      }
    }
  }
  path.push_back(v2);
  return path;
}

// Clobbers the incidence matrix and assumes graph is connected
bool Graph::verify_mst(const Matrix<double>& weights, Matrix<int>& incidence) {
  int num_edges = 0;
  std::vector<std::vector<int>> mst_incidence(incidence.size(), std::vector<int>(incidence.size(), 0));
  for (int v1 = 0; v1 < edges_.size(); v1++) {
    const std::vector<int>& edge = edges_[v1];
    for (int j = 0; j < edge.size(); j++) {
      num_edges++;
      int v2 = edge[j];
      incidence.set(v1, v2, 0);
      mst_incidence[v1][v2] = 1;
    }
  }
  CHECK_EQ(num_edges, 2*(n_ - 1));

  // Only iterate through the edges not in the graph
  for (int i = 0; i < incidence.size(); i++) {
    for (int j = 0; j <= i; j++) {
      if (incidence.is_set(i, j) && incidence.get(i, j)) {
        std::vector<int> path = find_path(i, j, mst_incidence);
        double weight = weights.get(i, j);
        for (int k = 0; k < path.size() - 1; k++) {
          int v1 = path[k];
          int v2 = path[k+1];
          if (weights.get(v1, v2) > weight) {
            return false;
          }
        }
      }
    }
  }
  return true;
}


}  // namespace math_opt_benchmark

