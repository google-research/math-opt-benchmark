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

#include "absl/random/random.h"
#include "gflags/gflags.h"
#include "math_opt_benchmark/mst/graph/graph.h"
#include "math_opt_benchmark/mst/mst.h"
#include "ortools/linear_solver/linear_solver.h"
#include "ortools/graph/max_flow.h"

DEFINE_int32(num_vars, 10, "How many variables are in the problem.");

int num_vars_flag() { return FLAGS_num_vars; }

constexpr double kTolerance = 1e-5;

namespace math_opt_benchmark {

void PrintSolution(const MSTSolution& solution) {
  std::cout << "Solution objective: " << solution.objective_value << std::endl;
  for (auto& x_vector : solution.x_values.as_vector_vector()) {
    std::cout << "Solution variable values: " << absl::StrJoin(x_vector, ",")
              << std::endl;
  }
}

template <class T>
void PrintVector(std::vector<T> vec) {
  std::cout << "Vector: " << absl::StrJoin(vec, ",") << std::endl;
}

/**
 * Construct graph from edges with non-zero solution values
 * @param problem Graph specification with edges, weights, and number of
 * vertices
 * @param solution Solution values from LP solve routine
 */
Graph toGraph(const MSTProblem& problem, const MSTSolution& solution) {
  std::vector<std::vector<int>> edges(problem.n);
  for (int v1 = 0; v1 < problem.n; ++v1) {
    for (const int& v2 : problem.edges.as_vector(v1)) {
      if (std::abs(solution.x_values.get(v1, v2)) > kTolerance) {
        edges[v1].push_back(v2);
        edges[v2].push_back(v1);
      }
    }
  }
  Graph g(std::move(edges));
  return g;
}

void MSTMain() {
  const int N = num_vars_flag();
  absl::BitGen gen;

  MSTProblem problem;
  problem.n = N;
  problem.edges.init(N);
  problem.weights.init(N);
  // Complete graph with weights ~ Unif(0, 1)
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < i; j++) {
      problem.edges.set(i, j, j);
      problem.weights.set(i, j, absl::Uniform(gen, 0.0, 1.0));
    }
  }
  MSTSolver solver(operations_research::MPSolver::SCIP_MIXED_INTEGER_PROGRAMMING, problem);
  MSTSolution solution = solver.Solve();

  Graph graph = toGraph(problem, solution);
  std::vector<std::vector<int>> invalid = graph.invalid_components(solution.x_values);

  std::vector<std::vector<int>> last;
  while (!invalid.empty() && invalid != last) {
    last = invalid;
    solver.AddConstraints(problem, invalid);
    solution = solver.Solve();
    graph = toGraph(problem, solution);
    invalid = graph.invalid_components(solution.x_values);
  }

  PrintSolution(solution);
}

}  // namespace math_opt_benchmark

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  math_opt_benchmark::MSTMain();
  return 0;
}