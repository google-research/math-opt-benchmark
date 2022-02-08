// Copyright 2021 The MathOpt Benchmark Authors.
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

#include <iostream>
#include <string>

#include "gflags/gflags.h"
#include "absl/random/random.h"
#include "math_opt_benchmark/example/example.h"
#include "ortools/linear_solver/linear_solver.h"

DEFINE_int32(
          num_vars, 10, "How many variables are in the problem.");
DEFINE_double(
          rhs, 4.0, "How many variables can be selected.");
DEFINE_bool(
          use_integers, false, "If the variables should be integer.");
DEFINE_string(solver, "glop", "The solver to use. Set to \"scip\" or \"glop\"");

int num_vars_flag() {
  return FLAGS_num_vars;
}

double rhs_flag() {
  return FLAGS_rhs;
}

bool use_integers_flag() {
  return FLAGS_use_integers;
}
operations_research::MPSolver::OptimizationProblemType solver_flag() {
  if(FLAGS_solver == "scip") {
    return operations_research::MPSolver::SCIP_MIXED_INTEGER_PROGRAMMING;
  }
  return operations_research::MPSolver::GLOP_LINEAR_PROGRAMMING;
}

namespace math_opt_benchmark {

void PrintSolution(const ExampleSolution& solution) {
  std::cout << "Solution objective: " << solution.objective_value << std::endl;
  std::cout << "Solution variable values: "
            << absl::StrJoin(solution.x_values, ",") << std::endl;
}

void RealMain() {
  ExampleProblem problem;
  problem.rhs = rhs_flag();
  problem.integer = use_integers_flag();
  const int num_vars = num_vars_flag();
  absl::BitGen gen;
  for (int i = 0; i < num_vars; ++i) {
    problem.objective.push_back(absl::Uniform(gen, 0.0, 1.0));
  }
  std::cout << "Objective coefficeints: ["
            << absl::StrJoin(problem.objective, ",") << "]" << std::endl;
  ExampleSolver solver(solver_flag(), problem);
  PrintSolution(solver.Solve());
  std::cout << "Zeroing objective for first half of variables" << std::endl;
  for (int i = 0; i < num_vars / 2; ++i) {
    solver.UpdateObjective(i, 0.0);
  }
  PrintSolution(solver.Solve());
}

}  // namespace math_opt_benchmark

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  math_opt_benchmark::RealMain();
  return 0;
}
