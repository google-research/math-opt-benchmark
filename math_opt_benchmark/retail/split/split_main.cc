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


#include "split.h"

#include "absl/random/random.h"
#include "absl/flags/flag.h"
#include "ortools/linear_solver/linear_solver.h"
#include "absl/strings/str_join.h"


//DEFINE_bool(iterative, true);

//bool iteratve_flag() { return FLAGS_iterative; }

namespace math_opt_benchmark {

template <class T>
void PrintVector(std::vector<T> vec, const std::string& name="Vector") {
  std::cout << name << ": " << absl::StrJoin(vec, ",") << std::endl;
}

void PrintSolution(const SplitAndSubSolution& solution) {
  std::cout << "Solution objective: " << solution.objective_value << std::endl;
  PrintVector(solution.in_assortment, "Solution in assortment");
  PrintVector(solution.must_split, "Solution must split");
}

// Returns vector of indices indicating which y(i,j) is nonzero for each customer
std::vector<int> solve_dual(const SplitAndSubSolution& solution, const SplitAndSubProblem& problem) {
  std::vector<int> ys(problem.customer_orders.size(), -1);
//  #pragma omp parallel for schedule(dynamic, 5)
  for (int i = 0; i < problem.customer_orders.size(); i++) {
    const std::vector<int>& order = problem.customer_orders[i];
    for (int j : order) {
      if (!solution.in_assortment[j]) {
        ys[i] = j;
        break;
      }
    }
  }
  return ys;
}

std::vector<double> solve_primal(const SplitAndSubSolution& solution, const SplitAndSubProblem& problem) {
  std::vector<double> ss(problem.customer_orders.size(), 0);
//  #pragma omp parallel for schedule(dynamic, 5)
  for (int i = 0; i < problem.customer_orders.size(); i++) {
    const std::vector<int>& order = problem.customer_orders[i];
    for (int j : order) {
      if (!solution.in_assortment[j]) {
        ss[i] = 1;
        break;
      }
    }
  }
  return ss;
}

SplitAndSubSolution benders(SplitAndSubSolver& solver, const SplitAndSubProblem& problem) {
  SplitAndSubSolution solution = solver.Solve();
  double prev_obj = -1.0;
  while (prev_obj != solution.objective_value)
  {
    prev_obj = solution.objective_value;
    std::vector<int> ys = solve_dual(solution, problem);
    solver.AddBenderCut(ys);
    solution = solver.Solve();
  }
  solution.must_split = solve_primal(solution, problem);
  return solution;
}

void SplitMain() {
  SplitAndSubProblem problem;
  for (int i = 0; i < 3; i++) {
    problem.weights.push_back(i);
  }
  problem.customer_orders.push_back(std::vector<int>({0, 1}));
  problem.customer_orders.push_back(std::vector<int>({1, 2}));
  problem.capacity = 2;

  SplitAndSubSolver solver(operations_research::MPSolver::SCIP_MIXED_INTEGER_PROGRAMMING, problem);
  SplitAndSubSolution solution = benders(solver, problem);
  PrintSolution(solution);

  SplitAndSubSolver direct_solver(operations_research::MPSolver::SCIP_MIXED_INTEGER_PROGRAMMING, problem, false);
  SplitAndSubSolution direct_solution = direct_solver.Solve();
  PrintSolution(direct_solution);
}

} // namespace math_opt_benchmark

int main(int argc, char *argv[]) {
//  absl::ParseCommandLineFlags(&argc, &argv, true);
//  std::cerr << filename_flag() << std::endl;
  math_opt_benchmark::SplitMain();
}
