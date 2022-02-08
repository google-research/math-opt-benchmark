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

#include "math_opt_benchmark/example/example.h"

#include <limits>

#include "glog/logging.h"
#include "absl/strings/str_cat.h"

namespace math_opt_benchmark {

using ::operations_research::MPConstraint;
using ::operations_research::MPSolver;
using ::operations_research::MPVariable;

constexpr double kInf = std::numeric_limits<double>::infinity();

ExampleSolver::ExampleSolver(
    const MPSolver::OptimizationProblemType problem_type,
    const ExampleProblem& problem)
    : solver_("example_solver", problem_type) {
  solver_.MutableObjective()->SetMaximization();
  MPConstraint* c = solver_.MakeRowConstraint(-kInf, problem.rhs);
  for (int i = 0; i < problem.objective.size(); ++i) {
    MPVariable* var =
        solver_.MakeVar(0.0, 1.0, problem.integer, absl::StrCat("x", i));
    x_vars_.push_back(var);
    solver_.MutableObjective()->SetCoefficient(var, problem.objective[i]);
    c->SetCoefficient(var, 1.0);
  }
}

ExampleSolution ExampleSolver::Solve() {
  MPSolver::ResultStatus status = solver_.Solve();
  CHECK_EQ(status, MPSolver::OPTIMAL);
  ExampleSolution result;
  result.objective_value = solver_.Objective().Value();
  for (MPVariable* v : x_vars_) {
    result.x_values.push_back(v->solution_value());
  }
  return result;
}
void ExampleSolver::UpdateObjective(int index, double value) {
  CHECK_GE(index, 0);
  CHECK_LT(index, x_vars_.size());
  solver_.MutableObjective()->SetCoefficient(x_vars_[index], value);
}

}  // namespace math_opt_benchmark
