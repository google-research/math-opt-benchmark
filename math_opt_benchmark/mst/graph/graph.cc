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

#include "math_opt_benchmark/mst/graph/graph.h"

#include <cmath>

#include "absl/random/random.h"

constexpr double kTolerance = 1e-5;

namespace math_opt_benchmark {

void debug_graph(std::vector<std::vector<int>> edges) {
  for (auto& vector : edges) {
    std::cout << "[D] Graph: " << absl::StrJoin(vector, ",") << std::endl;
  }
}

/**
 * Construct graph from edges with non-zero solution values
 * @param problem Graph specification with edges, weights, and number of
 * vertices
 * @param solution Solution values from LP solve routine
 */
Graph::Graph(const MSTProblem& problem, const MSTSolution& solution) {
  n_ = problem.n;
  for (int v1 = 0; v1 < n_; ++v1) {
    edges_.emplace_back(0);
  }
  for (int v1 = 0; v1 < n_; ++v1) {
//    for (const int& v2 : problem.edges[v1]) {
    for (int v2 = 0; v2 < n_; ++v2) {
      if (std::abs(solution.x_values.get(v1, v2)) > kTolerance) {
        edges_[v1].push_back(v2);
        edges_[v2].push_back(v1);
      }
    }
  }
}

/**
 * Find cuts with no crossing edges
 * @param solution Solution values from LP solve routine
 * @return Vector of cuts
 */
std::vector<std::vector<int>> Graph::invalid_components(
    const MSTSolution& solution) {
  std::vector<bool> visited(n_, false);
  int num_visited = 0;
  std::vector<std::vector<int>> components;
  std::vector<int> stack;
  while (num_visited < n_) {
    components.emplace_back(0);
    std::vector<int> component;
    int index = n_;
    for (int i = 0; i < n_; ++i) {
      if (!visited[i]) {
        index = i;
        break;
      }
    }
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

  std::vector<std::vector<int>> invalid;
  for (const std::vector<int>& component : components) {
    double sum = 0;
    for (const int& v1 : component) {
      for (const int& v2 : edges_[v1]) {
        sum += solution.x_values.get(v1, v2);
      }
    }
    // Edges are counted twice in the graph
    if (sum / 2 > component.size() - 1 + kTolerance) {
      invalid.push_back(component);
    }
  }
  return invalid;
}

}  // namespace math_opt_benchmark