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

DEFINE_int32(num_vars, 10, "How many variables are in the problem.");

int num_vars_flag() { return FLAGS_num_vars; }

namespace math_opt_benchmark {

bool v_neq(std::vector<std::vector<int>> v1, std::vector<std::vector<int>> v2) {
  for (int i = 0; i < v1.size(); i++) {
    if (v1[i] != v2[i]) {
      return true;
    }
  }
  return false;
}

void PrintSolution(const MSTSolution& solution) {
  std::cout << "Done" << std::endl;
//  std::cout << "Solution objective: " << solution.objective_value << std::endl;
//  for (auto& x_vector : solution.x_values) {
//    std::cout << "Solution variable values: " << absl::StrJoin(x_vector, ",")
//              << std::endl;
//  }
}

void MSTMain() {
  MSTProblem problem;
  absl::BitGen gen;
  const int N = num_vars_flag();
  problem.n = N;
  problem.edges.init(N);
  problem.weights.init(N);
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      if (i != j) {
        problem.edges.set(i, j, j);
        problem.weights.set(i, j, absl::Uniform(gen, 0.0, 1.0));
      }
    }
  }
  MSTSolver solver(
      operations_research::MPSolver::SCIP_MIXED_INTEGER_PROGRAMMING, problem);
  MSTSolution solution = solver.Solve();
  Graph graph(problem, solution);
  std::vector<std::vector<int>> invalid = graph.invalid_components(solution);
  std::vector<std::vector<int>> last;
  while (!invalid.empty() &&
         (invalid.size() != last.size() || v_neq(invalid, last))) {
    last = invalid;
    solver.AddConstraints(problem, invalid);
    solution = solver.Solve();
    graph = Graph(problem, solution);
    invalid = graph.invalid_components(solution);
  }
  PrintSolution(solution);
}

}  // namespace math_opt_benchmark

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  math_opt_benchmark::MSTMain();
  return 0;
}