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


#include "ufl.h"
#include "knapsack.h"

#include "absl/random/random.h"
#include "gflags/gflags.h"
#include "ortools/linear_solver/linear_solver.h"

DEFINE_int32(num_vars, 3, "How many variables are in the problem.");

int num_vars_flag() { return FLAGS_num_vars; }

namespace math_opt_benchmark {

UFLSolution benders(UFLSolver& solver, std::vector<std::vector<double>>& supply_costs,
                    std::vector<std::vector<int>>& cost_indices, int num_customers, int num_facilities) {
  UFLSolution solution = solver.Solve();
  double prev_objective = -1.0;
  while (solution.objective_value != prev_objective) {
    std::vector<double> y_coefficients(num_facilities, 0.0);
    double sum = 0.0;
    for (int i = 0; i < num_customers; i++) {
      std::vector<int> &indices = cost_indices[i];
      std::vector<double> &costs = supply_costs[i];
      std::vector<double> y_solution(num_facilities);
      for (int j = 0; j < num_facilities; j++) {
        y_solution[j] = solution.open_values[indices[j]];
      }
      // Don't actually need to solve the knapsack, just need the length
      std::vector<double> knapsack_solution = knapsack(costs, y_solution);
      int k = knapsack_solution.size();
      sum += costs[k - 1];
      for (int j = 0; j < k - 1; j++) {
        y_coefficients[indices[j]] += costs[k - 1] - costs[j];
      }
    }
    solver.AddBenderCut(sum, y_coefficients);
    solution = solver.Solve();
  }
  return solution;
}

void UFLMain() {
  const int N = num_vars_flag();
  absl::BitGen gen;

  UFLProblem problem;
  problem.num_facilities = 3;
  problem.num_customers = 3;
  problem.open_costs.reserve(N);
  problem.supply_costs = std::vector<std::vector<double>>(problem.num_customers, std::vector<double>(problem.num_facilities));
  for (int i = 0; i < N; i++) {
    problem.open_costs.push_back(i);
    for (int j = 0; j < N; j++) {
      problem.supply_costs[i][j] = absl::Uniform(gen, 0.0, 1.0);
    }
  }
  std::vector<std::vector<int>> sorted_cost_indices(problem.num_customers, std::vector<int>(problem.num_facilities));
  for (int i = 0; i < problem.num_customers; i++) {
    std::vector<double>& costs = problem.supply_costs[i];
    std::vector<int>& indices = sorted_cost_indices[i];
    std::sort(costs.begin(), costs.end(), [](double i, double j) { return i < j; });
    std::sort(indices.begin(), indices.end(), [&, costs](int i, int j) { return costs[i] < costs[j]; });
  }
  UFLSolver solver(operations_research::MPSolver::SCIP_MIXED_INTEGER_PROGRAMMING, problem);
  UFLSolution solution = benders(solver, problem.supply_costs, sorted_cost_indices, problem.num_customers, problem.num_facilities);
  solver.EnforceInteger();
  solution = benders(solver, problem.supply_costs, sorted_cost_indices, problem.num_customers, problem.num_facilities);

}

} // namespace math_opt_benchmark

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  math_opt_benchmark::UFLMain();
}