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
  supply_vars_.init(problem.num_customers, problem.num_facilities);
  for (int i = 0; i < problem.num_customers; ++i) {
    for (int j = 0; j < problem.num_facilities; ++j) {
      MPVariable *var = solver_.MakeVar(0.0, 1.0, false, absl::StrCat("x", i, j));
      supply_vars_.set(i, j, var);
      UpdateObjective(i, j, problem.supply_costs.get(i, j));
    }
  }
  for (int i = 0; i < problem.num_facilities; ++i) {
    MPVariable *var = solver_.MakeVar(0.0, 1.0, problem.integer, absl::StrCat("y", i));
    open_vars_.push_back(var);
    UpdateObjective(i, problem.open_costs[i]);
  }
  // Feasibility constraint: at least one facility open
  MPConstraint *c_eq = solver_.MakeRowConstraint(1, kInf);
  for (int i = 0; i < problem.num_facilities; ++i) {
    c_eq->SetCoefficient(open_vars_[i], 1);
  }
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
  std::pair<int, int> size = supply_vars_.size();
  result.supply_values.init(size.first, size.second);
  for (int i = 0; i < size.first; i++) {
    for (int j = 0; j < size.second; j++) {
      result.supply_values.set(i, j, supply_vars_.get(i, j)->solution_value());
    }
  }
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
void UFLSolver::UpdateObjective(int facility, int customer, double value) {
  CHECK_GE(facility, 0);
  CHECK_GE(customer, 0);
  CHECK_LT(facility, supply_vars_.num_rows());
  CHECK_LT(customer, supply_vars_.num_cols());
  solver_.MutableObjective()->SetCoefficient(supply_vars_.get(facility, customer), value);
}

void UFLSolver::UpdateObjective(int facility, double value) {
  CHECK_GE(facility, 0);
  CHECK_LT(facility, open_vars_.size());
  solver_.MutableObjective()->SetCoefficient(open_vars_[facility], value);
}

int sort_by_size(const std::vector<int> &a, const std::vector<int> &b) {
  return a.size() < b.size();
}

/**
 * Adds a Benders cut for each
 * @param problem
 * @param invalid
 */
/*
void UFLSolver::AddConstraints(const UFLProblem &problem,
                               std::vector<std::vector<int>> invalid) {
  constexpr int kMaxNewConstraints = 25;
  std::sort(invalid.begin(), invalid.end(), sort_by_size);
  for (int i = 0; i < invalid.size() && i < kMaxNewConstraints; ++i) {
    MPConstraint *c = solver_.MakeRowConstraint(-kInf, (double)invalid[i].size() - 1);
    for (const int j : invalid[i]) {
      if (i != j) {
        c->SetCoefficient(x_vars_.get(i, j), 1);
      }
    }
  }
}
*/
}