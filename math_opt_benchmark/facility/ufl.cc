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

constexpr double kInf = std::numeric_limits<double>::infinity();

namespace math_opt_benchmark {

namespace math_opt = operations_research::math_opt;

/**
 * @param problem_type MPSolver constant specifying which solver to use
 * @param problem Facility location specification with costs and sizes
 */
UFLSolver::UFLSolver(math_opt::SolverType problem_type, const UFLProblem &problem, bool iterative=true)
    : model_("UFL Solver"),
      bender_var_(model_.AddContinuousVariable(0.0, kInf, "w")) {
  solver_ = math_opt::IncrementalSolver::New(model_, problem_type).value();
  update_tracker_ = model_.NewUpdateTracker();
  model_.set_minimize();
  supply_vars_.reserve(problem.num_customers);
  bool integer = !iterative;
  for (int i = 0; i < problem.num_customers; ++i) {
    supply_vars_.emplace_back();
    supply_vars_[i].reserve(problem.num_facilities);
    for (int j = 0; j < problem.num_facilities; ++j) {
      math_opt::Variable var = model_.AddContinuousVariable(0.0, 1.0, absl::StrCat("x", i, ",", j));
      supply_vars_[i].push_back(var);
    }
  }

  open_vars_.reserve(problem.num_facilities);
  for (int i = 0; i < problem.num_facilities; ++i) {
    math_opt::Variable var = model_.AddVariable(0.0, 1.0, integer, absl::StrCat("y", i));
    open_vars_.push_back(var);
    model_.set_objective_coefficient(var, problem.open_costs[i]);
  }
  // Feasibility constraint: at least one facility open
  const math_opt::LinearConstraint feasible = model_.AddLinearConstraint(1, kInf);
  for (int i = 0; i < problem.num_facilities; ++i) {
    model_.set_coefficient(feasible, open_vars_[i], 1);
  }

  model_.set_objective_coefficient(bender_var_, 1);

  // Regular problem formulation
  if (!iterative) {
    // Minimize customer costs
    for (int i = 0; i < problem.num_customers; ++i) {
      for (int j = 0; j < problem.num_facilities; ++j) {
        model_.set_objective_coefficient(supply_vars_[i][j], problem.supply_costs[i][j]);
      }
    }

    for (int i = 0; i < problem.num_facilities; ++i) {
      math_opt::LinearConstraint c = model_.AddLinearConstraint(0, 1);
      model_.set_coefficient(c, open_vars_[i], 1);
    }

    for (int i = 0; i < problem.num_customers; ++i) {
      // Each customer is fulfilled
      math_opt::LinearConstraint full = model_.AddLinearConstraint(1, 1);
      for (int j = 0; j < problem.num_facilities; ++j) {
        model_.set_coefficient(full, supply_vars_[i][j], 1);
        // Only supplied by open facilities
        math_opt::LinearConstraint open = model_.AddLinearConstraint(-kInf, 0);
        model_.set_coefficient(open, supply_vars_[i][j], 1);
        model_.set_coefficient(open, open_vars_[j], -1);
      }
    }
  }
   *(instance_.mutable_initial_model()) = model_.ExportModel();
}


/**
 * Calls the MPSolver solve routine and stores the result in a solution object
 * @return Solution containing objective value and variable values
 */
UFLSolution UFLSolver::Solve() {
  math_opt::SolveArguments solve_args;
  absl::StatusOr<math_opt::SolveResult> result = solver_->Solve(solve_args);
  CHECK_EQ(result.value().termination.reason, math_opt::TerminationReason::kOptimal) << result.value().termination.detail;

  UFLSolution solution;
  solution.objective_value = result.value().objective_value();
  solution.open_values.reserve(open_vars_.size());
  for (math_opt::Variable v : open_vars_) {
    solution.open_values.push_back(result.value().variable_values().at(v));
  }

  instance_.add_objectives(solution.objective_value);
  return solution;
}

int sort_by_size(const std::vector<int> &a, const std::vector<int> &b) {
  return a.size() < b.size();
}

void UFLSolver::AddBenderCut(double sum, const std::vector<double> &y_coefficients) {
  // bender_var_ >= sum - \sum(y * y_coefficients)

  update_tracker_->Checkpoint();
  math_opt::LinearConstraint cut = model_.AddLinearConstraint(sum, kInf);
  model_.set_coefficient(cut, bender_var_, 1);
  for (int i = 0; i < open_vars_.size(); i++) {
    model_.set_coefficient(cut, open_vars_[i], y_coefficients[i]);
  }
  *(instance_.add_model_updates()) = *update_tracker_->ExportModelUpdate();
}

void UFLSolver::EnforceInteger() {
  for (math_opt::Variable v : open_vars_) {
    model_.set_is_integer(v, true);
  }
}

BenchmarkInstance UFLSolver::GetModel() {
  return instance_;
}

} // namespace math_opt_benchmark