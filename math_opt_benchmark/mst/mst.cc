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

#include "math_opt_benchmark/mst/mst.h"

#include <limits>

#include "absl/strings/str_cat.h"
#include "glog/logging.h"

namespace math_opt_benchmark {

using ::operations_research::MPConstraint;
using ::operations_research::MPSolver;
using ::operations_research::MPVariable;

/**
 * @param problem_type MPSolver constant specifying which solver to use
 * @param problem Graph specification with edges, weights, and number of
 * vertices
 */
MSTSolver::MSTSolver(const MPSolver::OptimizationProblemType problem_type,
                     const MSTProblem &problem)
    : solver_("MST_solver", problem_type) {
  solver_.MutableObjective()->SetMinimization();
  for (int i = 0; i < problem.n; ++i) {
    x_vars_.push_back(
        std::vector<operations_research::MPVariable *>(problem.n, nullptr));
  }
  for (int i = 0; i < problem.n; ++i) {
    for (int j = i; j < problem.n; ++j) {
      MPVariable *var =
          solver_.MakeVar(0.0, 1.0, problem.integer, absl::StrCat("x", i, j));
      x_vars_[i][j] = x_vars_[j][i] = var;
      UpdateObjective(i, j, problem.weights[i][j]);
    }
  }
  for (int i = 0; i < problem.n; ++i) {
    MPConstraint *c = solver_.MakeRowConstraint(0, 0);
    c->SetCoefficient(x_vars_[i][i], 1);
  }
  MPConstraint *c_eq = solver_.MakeRowConstraint(problem.n - 1, problem.n - 1);
  for (int i = 0; i < problem.n; ++i) {
    for (int j = i + 1; j < problem.n; ++j) {
      c_eq->SetCoefficient(x_vars_[i][j], 1);
    }
  }
}

void debug_solve(const MSTSolution &result) {
  for (int i = 0; i < result.x_values.size(); i++) {
    for (int j = 0; j < result.x_values[i].size(); j++) {
      printf("[D] x[%i][%i] = %.7f\n", i, j, result.x_values[i][j]);
    }
  }
}

/**
 * Calls the MPSolver solve routine and stores the result in a solution object
 * @return Solution containing objective value and variable values
 */
MSTSolution MSTSolver::Solve() {
  MPSolver::ResultStatus status = solver_.Solve();
  CHECK_EQ(status, MPSolver::OPTIMAL);
  MSTSolution result;
  result.objective_value = solver_.Objective().Value();
  result.x_values = std::vector<std::vector<double>>(
      x_vars_.size(), std::vector<double>(x_vars_.size(), 0));
  for (int i = 0; i < x_vars_.size(); i++) {
    for (int j = 0; j < x_vars_.size(); j++) {
      result.x_values[i][j] = (x_vars_[i][j]->solution_value());
    }
  }
  return result;
}

/**
 * Verifies input and updates the coefficient for a variable
 * @param v1 Vertex 1
 * @param v2 Vertex 2
 * @param value Variable coefficient
 */
void MSTSolver::UpdateObjective(int v1, int v2, double value) {
  CHECK_GE(v1, 0);
  CHECK_GE(v2, 0);
  CHECK_LT(v1, x_vars_.size());
  CHECK_LT(v2, x_vars_[v1].size());
  solver_.MutableObjective()->SetCoefficient(x_vars_[v1][v2], value);
}

/**
 *
 * @param problem Graph specification with edges, weights, and number of
 * vertices
 * @param invalid List of lists of vertices that violate the MST constraint
 */
void MSTSolver::AddConstraints(const MSTProblem &problem,
                               std::vector<std::vector<int>> invalid) {
  static constexpr int max_new_constraints = 25;
  for (int i = 0; i < invalid.size() && i < max_new_constraints; ++i) {
    MPConstraint *c = solver_.MakeRowConstraint(0, invalid[i].size() - 1);
    for (const int &j : invalid[i]) {
      c->SetCoefficient(x_vars_[i][j], 1);
    }
  }
}

}  // namespace math_opt_benchmark
