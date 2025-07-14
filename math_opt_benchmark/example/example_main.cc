// Copyright 2025 The MathOpt Benchmark Authors.
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

#include "ortools/base/init_google.h"
#include "absl/flags/flag.h"
#include "absl/random/random.h"
#include "absl/strings/str_join.h"
#include "math_opt_benchmark/example/example.h"
#include "ortools/math_opt/cpp/math_opt.h"

ABSL_FLAG(int, num_vars, 10, "How many variables are in the problem.");
ABSL_FLAG(double, rhs, 4.0, "How many variables can be selected.");
ABSL_FLAG(bool, use_integers, false, "If the variables should be integer.");

ABSL_FLAG(operations_research::math_opt::SolverType, solver,
          operations_research::math_opt::SolverType::kGlop,
          "The solver to use. Set to \"gscip\" or \"glop\"");

namespace math_opt_benchmark {
namespace {

void PrintSolution(const ExampleSolution& solution) {
  std::cout << "Solution objective: " << solution.objective_value << std::endl;
  std::cout << "Solution variable values: "
            << absl::StrJoin(solution.x_values, ",") << std::endl;
}

void Main() {
  ExampleProblem problem;
  problem.rhs = absl::GetFlag(FLAGS_rhs);
  problem.integer = absl::GetFlag(FLAGS_use_integers);
  const int num_vars = absl::GetFlag(FLAGS_num_vars);
  absl::BitGen gen;
  for (int i = 0; i < num_vars; ++i) {
    problem.objective.push_back(absl::Uniform(gen, 0.0, 1.0));
  }
  std::cout << "Objective coefficeints: ["
            << absl::StrJoin(problem.objective, ",") << "]" << std::endl;
  ExampleSolver solver(absl::GetFlag(FLAGS_solver), problem);
  PrintSolution(solver.Solve());
  std::cout << "Zeroing objective for first half of variables" << std::endl;
  for (int i = 0; i < num_vars / 2; ++i) {
    solver.UpdateObjective(i, 0.0);
  }
  PrintSolution(solver.Solve());
}

}  // namespace
}  // namespace math_opt_benchmark

int main(int argc, char** argv) {
  InitGoogle(argv[0], &argc, &argv, true);
  math_opt_benchmark::Main();
  return 0;
}
