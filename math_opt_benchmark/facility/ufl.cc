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
    : optimizer_(problem_type, "UFL Solver"),
    bender_var_(optimizer_.AddContinuousVariable(0.0, kInf, "w")) {
  optimizer_.objective().set_minimize();
  supply_vars_.reserve(problem.num_customers);
  bool integer = !iterative;
  for (int i = 0; i < problem.num_customers; ++i) {
    supply_vars_.emplace_back();
    supply_vars_[i].reserve(problem.num_facilities);
    for (int j = 0; j < problem.num_facilities; ++j) {
      math_opt::Variable var = optimizer_.AddContinuousVariable(0.0, 1.0, absl::StrCat("x", i, ",", j));
      supply_vars_[i].push_back(var);
    }
  }

  open_vars_.reserve(problem.num_facilities);
  for (int i = 0; i < problem.num_facilities; ++i) {
    math_opt::Variable var = optimizer_.AddVariable(0.0, 1.0, integer, absl::StrCat("y", i));
    open_vars_.push_back(var);
    UpdateObjective(var, problem.open_costs[i]);
  }
  // Feasibility constraint: at least one facility open
  const math_opt::LinearConstraint feasible = optimizer_.AddLinearConstraint(1, kInf);
  for (int i = 0; i < problem.num_facilities; ++i) {
    feasible.set_coefficient(open_vars_[i], 1);
  }

  UpdateObjective(bender_var_, 1);

  // Regular problem formulation
  if (!iterative) {
    // Minimize customer costs
    for (int i = 0; i < problem.num_customers; ++i) {
      for (int j = 0; j < problem.num_facilities; ++j) {
        UpdateObjective(supply_vars_[i][j], problem.supply_costs[i][j]);
      }
    }

    for (int i = 0; i < problem.num_facilities; ++i) {
      math_opt::LinearConstraint c = optimizer_.AddLinearConstraint(0, 1);
      c.set_coefficient(open_vars_[i], 1);
    }

    for (int i = 0; i < problem.num_customers; ++i) {
      // Each customer is fulfilled
      math_opt::LinearConstraint full = optimizer_.AddLinearConstraint(1, 1);
      for (int j = 0; j < problem.num_facilities; ++j) {
        full.set_coefficient(supply_vars_[i][j], 1);
        // Only supplied by open facilities
        math_opt::LinearConstraint open = optimizer_.AddLinearConstraint(-kInf, 0);
        open.set_coefficient(supply_vars_[i][j], 1);
        open.set_coefficient(open_vars_[j], -1);
      }
    }
  }

   *(model_.mutable_initial_model()) = optimizer_.ExportModel();
}


/**
 * Calls the MPSolver solve routine and stores the result in a solution object
 * @return Solution containing objective value and variable values
 */
UFLSolution UFLSolver::Solve() {
  math_opt::SolveParametersProto solve_parameters;
  solve_parameters.mutable_common_parameters()->set_enable_output(false);
  absl::StatusOr<math_opt::Result> result = optimizer_.Solve(solve_parameters);
  CHECK_EQ(result.value().termination_reason, math_opt::SolveResultProto::OPTIMAL) << result.value().termination_detail;

  UFLSolution solution;
  solution.objective_value = result.value().objective_value();
  solution.open_values.reserve(open_vars_.size());
  for (math_opt::Variable v : open_vars_) {
    solution.open_values.push_back(result.value().variable_values().at(v));
  }

  model_.add_objectives(solution.objective_value);
  return solution;
}

/**
 * Verifies input and updates the coefficient for a variable
 * @param var MPVariable in the solver
 * @param value New variable coefficient
 */
void UFLSolver::UpdateObjective(math_opt::Variable var, double value) {
  optimizer_.objective().set_linear_coefficient(var, value);
}

int sort_by_size(const std::vector<int> &a, const std::vector<int> &b) {
  return a.size() < b.size();
}


void UFLSolver::AddBenderCut(double sum, const std::vector<double> &y_coefficients) {
  // bender_var_ >= sum - \sum(y * y_coefficients)
  math_opt::LinearConstraint cut = optimizer_.AddLinearConstraint(sum, kInf);
  cut.set_coefficient(bender_var_, 1);
  for (int i = 0; i < open_vars_.size(); i++) {
    cut.set_coefficient(open_vars_[i], y_coefficients[i]);
  }

  *(model_.add_model_updates()) = optimizer_.ExportModelUpdate();
}

void UFLSolver::EnforceInteger() {
  for (math_opt::Variable v : open_vars_) {
    v.set_is_integer(true);
  }
}

BenchmarkInstance UFLSolver::GetModel() {
  return model_;
}

} // namespace math_opt_benchmark