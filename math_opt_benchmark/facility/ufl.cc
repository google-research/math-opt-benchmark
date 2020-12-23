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

#include "absl/strings/str_cat.h"
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
UFLSolver::UFLSolver(operations_research::MPSolver::OptimizationProblemType problem_type, const UFLProblem &problem)
    : solver_("UFL_solver", problem_type) {
  solver_.MutableObjective()->SetMinimization();
  supply_vars_ = std::vector(problem.num_customers, std::vector<MPVariable *>(problem.num_facilities));
  for (int i = 0; i < problem.num_customers; ++i) {
    for (int j = 0; j < problem.num_facilities; ++j) {
      MPVariable *var = solver_.MakeVar(0.0, 1.0, false, absl::StrCat("x", i, j));
      supply_vars_[i][j] = var;
    }
  }
  open_vars_.reserve(problem.num_facilities);
  for (int i = 0; i < problem.num_facilities; ++i) {
    MPVariable *var = solver_.MakeVar(0.0, 1.0, problem.integer, absl::StrCat("y", i));
    open_vars_.push_back(var);
    UpdateObjective(var, problem.open_costs[i]);
  }
  // Feasibility constraint: at least one facility open
  MPConstraint *feasible = solver_.MakeRowConstraint(1, kInf);
  for (int i = 0; i < problem.num_facilities; ++i) {
    feasible->SetCoefficient(open_vars_[i], 1);
  }

  bender_var_ = solver_.MakeVar(0.0, kInf, false, "w");
  UpdateObjective(bender_var_, 1);
}


/*
void debug_solve(const UFLSolution &result) {
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
UFLSolution UFLSolver::Solve() {
  MPSolver::ResultStatus status = solver_.Solve();
  CHECK_EQ(status, MPSolver::OPTIMAL);
  UFLSolution result;
  result.objective_value = solver_.Objective().Value();
  int num_customers = supply_vars_.size();
  int num_facilities = supply_vars_[0].size();
  result.supply_values = std::vector<std::vector<double>>(num_customers, std::vector<double>(num_facilities));
  for (int i = 0; i < num_customers; i++) {
    for (int j = 0; j < num_facilities; j++) {
      result.supply_values[i][j] =  supply_vars_[i][j]->solution_value();
    }
  }
  result.open_values.reserve(open_vars_.size());
  for (const MPVariable *v : open_vars_) {
    result.open_values.push_back(v->solution_value());
  }
  return result;
}

/**
 * Verifies input and updates the coefficient for a variable
 * @param facility Facility index
 * @param customer Customer index
 * @param value Variable coefficient
 */
void UFLSolver::UpdateObjective(operations_research::MPVariable *var, double value) {
  solver_.MutableObjective()->SetCoefficient(var, value);
}

int sort_by_size(const std::vector<int> &a, const std::vector<int> &b) {
  return a.size() < b.size();
}

/**
 * Adds a Benders cut for each
 * @param problem
 * @param invalid
 */
void UFLSolver::AddBenderCut(double sum, const std::vector<double> &y_coefficients) {
  // bender_var_ >= sum - \sum(y * y_coefficients)
  MPConstraint *cut = solver_.MakeRowConstraint(sum, kInf);
  cut->SetCoefficient(bender_var_, 1);
  for (int i = 0; i < open_vars_.size(); i++) {
    cut->SetCoefficient(open_vars_[i], y_coefficients[i]);
  }
}
void UFLSolver::EnforceInteger() {
  for (MPVariable *v : open_vars_) {
    v->SetInteger(true);
  }
}

} // namespace math_opt_benchmark