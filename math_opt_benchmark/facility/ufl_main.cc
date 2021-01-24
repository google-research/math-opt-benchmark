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
//#include "knapsack.h"

#include "absl/random/random.h"
#include "gflags/gflags.h"
#include "ortools/linear_solver/linear_solver.h"

#include <fstream>

DEFINE_string(filename, "ORLIB/ORLIB-uncap/70/cap71.txt", "Path to ORLIB problem specification.");

std::string filename_flag() { return FLAGS_filename; }

namespace math_opt_benchmark {

template <class T>
void PrintVector(std::vector<T> vec, const std::string& name="Vector") {
  std::cout << name << ": " << absl::StrJoin(vec, ",") << std::endl;
}

void PrintSolution(const UFLSolution& solution) {
  std::cout << "Solution objective: " << solution.objective_value << std::endl;
  PrintVector(solution.open_values, "Solution open values");
  for (auto& x_vector : solution.supply_values) {
    PrintVector(x_vector, "Solution supply values:");
  }
}

UFLProblem ParseProblem(const std::string& name) {
  UFLProblem problem;
  std::ifstream file(name);
  std::string line;
  std::getline(file, line);
  std::istringstream lineTokens(line);
  lineTokens >> problem.num_facilities;
  lineTokens >> problem.num_customers;

  problem.open_costs = std::vector<double>(problem.num_facilities);
  std::string tmp;
  for (int i = 0; i < problem.num_facilities; i++) {
    std::getline(file, line);
    lineTokens = std::istringstream(line);
    lineTokens >> tmp;
    lineTokens >> problem.open_costs[i];
  }


  problem.supply_costs = std::vector<std::vector<double>>(problem.num_customers, std::vector<double>(problem.num_facilities));
  double cost = 0.0;
  for (int i = 0; i < problem.num_customers; i++) {
    // Skip demand
    std::getline(file, line);
    int parsed = 0;
    while (parsed < problem.num_facilities) {
      std::getline(file, line);
      lineTokens = std::istringstream(line);
      while (lineTokens >> cost) {
        problem.supply_costs[i][parsed] = cost;
        parsed++;
      }
    }
  }

  return problem;
}

// Assume costs and ys are sorted together so costs[i] < costs[i+1]
std::vector<double> knapsack(const std::vector<double>& costs, const std::vector<double>& ys) {
  std::vector<double> solution;
  double sum = 0;
  int k;
  for (k = 0; k < ys.size() && sum < 1; k++) {
    sum += ys[k];
  }
  solution.reserve(k);
  for (int i = 0; i < k-1; i++) {
    solution.push_back(ys[k]);
  }
  solution.push_back(1 - sum + ys[k-1]);

  return solution;
}

UFLSolution benders(UFLSolver& solver, std::vector<std::vector<double>>& supply_costs,
                    std::vector<std::vector<int>>& cost_indices, int num_customers, int num_facilities) {
  UFLSolution solution = solver.Solve();
//  PrintSolution(solution);
  double prev_objective = -1.0;
  while (solution.objective_value != prev_objective) {
    prev_objective = solution.objective_value;
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
//      PrintVector(knapsack_solution);
      int k = knapsack_solution.size();
      sum += costs[k - 1];
      for (int j = 0; j < k - 1; j++) {
        y_coefficients[indices[j]] += costs[k - 1] - costs[j];
      }
    }
//    PrintVector(y_coefficients);
    solver.AddBenderCut(sum, y_coefficients);
    solution = solver.Solve();
  }
  return solution;
}

void UFLMain() {
//  const int N = num_vars_flag();
//  absl::BitGen gen;
//
//  UFLProblem problem;
//  problem.num_facilities = 3;
//  problem.num_customers = 3;
//  problem.open_costs.reserve(N);
//  problem.supply_costs = std::vector<std::vector<double>>(problem.num_customers, std::vector<double>(problem.num_facilities));
//  for (int i = 0; i < N; i++) {
//    problem.open_costs.push_back(i);
//    for (int j = 0; j < N; j++) {
//      problem.supply_costs[i][j] = absl::Uniform(gen, 0.0, 1.0);
//    }
//  }
//  printf("Open\n");
//  PrintVector(problem.open_costs);
//  printf("Supply\n");
//  for (auto& v : problem.supply_costs) {
//    PrintVector(v);
//  }
//  printf("Indices\n");
  UFLProblem problem = ParseProblem(filename_flag());
  std::vector<std::vector<int>> sorted_cost_indices(problem.num_customers, std::vector<int>(problem.num_facilities));
  for (int i = 0; i < problem.num_customers; i++) {
    std::vector<double>& costs = problem.supply_costs[i];
    std::vector<int>& indices = sorted_cost_indices[i];
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&, costs](int i, int j) { return costs[i] < costs[j]; });
    std::sort(costs.begin(), costs.end(), [](double i, double j) { return i < j; });
//    PrintVector(indices);
  }
  UFLSolver solver(operations_research::MPSolver::SCIP_MIXED_INTEGER_PROGRAMMING, problem);
  UFLSolution solution = benders(solver, problem.supply_costs, sorted_cost_indices, problem.num_customers, problem.num_facilities);
  solver.EnforceInteger();
  solution = benders(solver, problem.supply_costs, sorted_cost_indices, problem.num_customers, problem.num_facilities);
  PrintSolution(solution);
}

} // namespace math_opt_benchmark

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  std::cout << filename_flag() << std::endl;
  math_opt_benchmark::UFLMain();
}