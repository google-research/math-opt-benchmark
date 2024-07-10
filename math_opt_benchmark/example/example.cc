// Copyright 2024 The MathOpt Benchmark Authors.
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

#include "math_opt_benchmark/example/example.h"

#include <limits>

#include "ortools/base/logging.h"  // logging.h
#include "absl/strings/str_cat.h"
#include "ortools/math_opt/cpp/math_opt.h"

namespace math_opt_benchmark {
namespace {
namespace math_opt = ::operations_research::math_opt;
constexpr double kInf = std::numeric_limits<double>::infinity();
}  // namespace

ExampleSolver::ExampleSolver(const math_opt::SolverType solver_type,
                             const ExampleProblem& problem)
    : model_("example_model") {
  for (int i = 0; i < problem.objective.size(); ++i) {
    x_vars_.push_back(
        model_.AddVariable(0.0, 1.0, problem.integer, absl::StrCat("x", i)));
  }
  model_.AddLinearConstraint(math_opt::Sum(x_vars_) <= problem.rhs);
  model_.Maximize(math_opt::InnerProduct(x_vars_, problem.objective));
  solver_ =
      math_opt::NewIncrementalSolver(
          model_,
          solver_type)
          .value();
}

ExampleSolution ExampleSolver::Solve() {
  const math_opt::SolveResult solve_result = solver_->Solve().value();
  CHECK_EQ(solve_result.termination.reason,
           math_opt::TerminationReason::kOptimal)
      << solve_result.termination;
  ExampleSolution result;
  result.objective_value = solve_result.objective_value();
  for (const math_opt::Variable x : x_vars_) {
    result.x_values.push_back(solve_result.variable_values().at(x));
  }
  return result;
}
void ExampleSolver::UpdateObjective(int index, double value) {
  model_.set_objective_coefficient(x_vars_.at(index), value);
}

}  // namespace math_opt_benchmark
