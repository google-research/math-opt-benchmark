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

#include "retail.h"

#include "absl/strings/str_cat.h"
#include "absl/random/random.h"
#include "glog/logging.h"

constexpr double kInf = std::numeric_limits<double>::infinity();

namespace math_opt_benchmark {

using ::operations_research::MPConstraint;
using ::operations_research::MPSolver;
using ::operations_research::MPVariable;

/**
 * @param problem_type MPSolver constant specifying which solver to use
 * @param problem Facility location specification with costs and sizes
 */
RetailSolver::RetailSolver(operations_research::MPSolver::OptimizationProblemType problem_type, const RetailProblem &problem)
    : solver_("Retail_solver", problem_type) {
  solver_.MutableObjective()->SetMinimization();
  bender_var_ = solver_.MakeVar(0.0, kInf, false, "z");
  UpdateObjective(bender_var_, 1);

  assortment_vars_.reserve(problem.weights.size());
  for (int i = 0; i < problem.weights.size(); i++) {
    MPVariable *var = solver_.MakeVar(0.0, 1.0, true, absl::StrCat("x", i));
    assortment_vars_.push_back(var);
  }

  // Sum of weights less than capacity
  MPConstraint *weight_limit = solver_.MakeRowConstraint(0, problem.capacity);
  for (int i = 0; i < problem.weights.size(); i++) {
    weight_limit->SetCoefficient(assortment_vars_[i], problem.weights[i]);
  }
}


/*
void debug_solve(const RetailSolution &result) {
  for (int i = 0; i < result.x_values.size(); i++) {
    for (int j = 0; j < result.x_values.size(); j++) {
      printf("[D] x[%i][%i] = %.7f\n", i, j, result.x_values.get(i, j));
    }
  }
}
*/

/**
 * Calls the MPSolver solve routine and stores the result in a solution object
 * @return Solution containing objective value and variable values
 */
RetailSolution RetailSolver::Solve() {
  MPSolver::ResultStatus status = solver_.Solve();
  CHECK_EQ(status, MPSolver::OPTIMAL);
  RetailSolution result;
  result.objective_value = solver_.Objective().Value();
  result.in_assortment.reserve(assortment_vars_.size());
  for (int i = 0; i < assortment_vars_.size(); i++) {
    result.in_assortment.push_back(assortment_vars_[i]->solution_value());
  }

  return result;
}

/**
 * Verifies input and updates the coefficient for a variable
 * @param var MPVariable in the solver
 * @param value New variable coefficient
 */
void RetailSolver::UpdateObjective(operations_research::MPVariable *var, double value) {
  solver_.MutableObjective()->SetCoefficient(var, value);
}

int sort_by_size(const std::vector<int> &a, const std::vector<int> &b) {
  return a.size() < b.size();
}

/**
 * Adds a Benders cut for each
 * @param sum
 * @param y_coefficients
 */
void RetailSolver::AddBenderCut(const std::vector<int>& y_indices) {
  int num_no_solutions = std::count(y_indices.begin(), y_indices.end(), -1);
  size_t num_solutions = y_indices.size() - num_no_solutions;
  MPConstraint *cut = solver_.MakeRowConstraint(num_solutions, kInf);
  for (int index : y_indices) {
    cut->SetCoefficient(assortment_vars_[index], 1);
  }
}

} // namespace math_opt_benchmark