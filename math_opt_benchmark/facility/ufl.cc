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


#include "math_opt_benchmark/facility/ufl.h"

#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "ortools/base/logging.h"  // status.h

constexpr double kInf = std::numeric_limits<double>::infinity();
namespace math_opt = operations_research::math_opt;

constexpr double kTolerance = 1e-5;

namespace math_opt_benchmark {

//
// UFLSolver
//

/**
 * @param solver_type Which solver to use
 * @param problem Facility location specification with costs and sizes
 */
UFLSolver::UFLSolver(math_opt::SolverType solver_type,
                     const UFLProblem& problem, bool iterative = true)
    : model_("UFL Solver"),
      bender_var_(model_.AddContinuousVariable(0.0, kInf, "w")),
      iterative_(iterative) {
  solver_ =
      math_opt::NewIncrementalSolver(
          model_,
          solver_type)
          .value();
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

  if (!iterative_) {
    for (int i = 0; i < supply_vars_.size(); i++) {
      for (int j = 0; j < supply_vars_[0].size(); j++) {
        if (result.value().variable_values().at(supply_vars_[i][j]) > 0.5) {
          solution.supply_values.push_back(j);
          break;
        }
      }
    }
  }

  instance_.add_objectives(solution.objective_value);
  return solution;
}

void UFLSolver::AddBenderCut(double sum, const std::vector<double> &y_coefficients) {
  // bender_var_ >= sum - \sum_i y_coefficients[i] * y_i
  update_tracker_->Checkpoint();

  math_opt::LinearConstraint cut = model_.AddLinearConstraint(sum, kInf);
  model_.set_coefficient(cut, bender_var_, 1);
  for (int i = 0; i < open_vars_.size(); i++) {
    model_.set_coefficient(cut, open_vars_[i], y_coefficients[i]);
  }
  std::optional<math_opt::ModelUpdateProto> update;
  update = update_tracker_
               ->ExportModelUpdate() ;
  if(update.has_value()){
    *(instance_.add_model_updates()) = *update;
  }
}

void UFLSolver::EnforceInteger() {
  for (math_opt::Variable v : open_vars_) {
    model_.set_is_integer(v, true);
  }
}

BenchmarkInstance UFLSolver::GetModel() {
  return instance_;
}

//
// UFLBenders
//

UFLBenders::UFLBenders(const UFLProblem& problem,
                       math_opt::SolverType solver_type)
    : problem_(problem),
      solver_(solver_type, problem, true),
      cost_indices_(problem.num_customers,
                    std::vector<int>(problem.num_facilities)) {
  for (int i = 0; i < problem_.num_customers; i++) {
    std::vector<double>& costs = problem_.supply_costs[i];
    std::vector<int>& indices = cost_indices_[i];
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(),
              [&, costs](int i, int j) { return costs[i] < costs[j]; });
    std::sort(costs.begin(), costs.end());
  }
}

UFLSolution UFLBenders::benders() {
  const int num_customers = problem_.num_customers;
  const int num_facilities = problem_.num_facilities;
  std::vector<std::vector<double>>& supply_costs = problem_.supply_costs;
  UFLSolution solution = solver_.Solve();
  double best_objective = solution.objective_value;
  double ub = kInf;
  while (ub - best_objective >= kTolerance) {
    std::vector<double> y_coefficients(num_facilities, 0.0);
    double sum = 0.0;
    for (int i = 0; i < num_customers; i++) {
      const std::vector<int>& indices = cost_indices_[i];
      const std::vector<double>& costs = supply_costs[i];
      std::vector<double> y_solution(num_facilities);
      for (int j = 0; j < num_facilities; j++) {
        y_solution[j] = solution.open_values[indices[j]];
      }
      // Don't actually need the knapsack solution, just need the length
      const int k = Knapsack(y_solution).size();
      sum += costs[k - 1];
      for (int j = 0; j < k - 1; j++) {
        y_coefficients[indices[j]] += costs[k - 1] - costs[j];
      }
    }
    double worker_obj = sum;
    for (int i = 0; i < num_facilities; i++) {
      worker_obj -= y_coefficients[i] * solution.open_values[i];
    }
    ub = std::min(ub, worker_obj);
    solver_.AddBenderCut(sum, y_coefficients);
    solution = solver_.Solve();
    best_objective = std::max(best_objective, solution.objective_value);
  }
  return solution;
}

UFLSolution UFLBenders::Solve() {
  UFLSolution solution = benders();
  solver_.EnforceInteger();
  solution = benders();
  solution.supply_values.reserve(problem_.num_customers);
  for (int i = 0; i < problem_.num_customers; i++) {
    int j;
    for (j = 0; j < problem_.num_facilities &&
                !solution.open_values[cost_indices_[i][j]];
         j++) {
    }
    solution.supply_values.push_back(cost_indices_[i][j]);
  }
  return solution;
}

//
// HELPER FUNCTIONS
//

UFLProblem ParseProblem(const std::string& contents) {
  UFLProblem problem;
  std::istringstream all_lines(contents);
  std::string line;
  std::getline(all_lines, line);
  std::istringstream lineTokens(line);
  lineTokens >> problem.num_facilities;
  lineTokens >> problem.num_customers;

  problem.open_costs = std::vector<double>(problem.num_facilities);
  std::string tmp;
  for (int i = 0; i < problem.num_facilities; i++) {
    std::getline(all_lines, line);
    lineTokens = std::istringstream(line);
    lineTokens >> tmp;  // Ignore the capacity
    lineTokens >> problem.open_costs[i];
  }

  problem.supply_costs = std::vector<std::vector<double>>(
      problem.num_customers, std::vector<double>(problem.num_facilities));
  double cost = 0.0;
  for (int i = 0; i < problem.num_customers; i++) {
    // Skip demand
    std::getline(all_lines, line);
    int parsed = 0;
    while (parsed < problem.num_facilities) {
      std::getline(all_lines, line);
      lineTokens = std::istringstream(line);
      while (lineTokens >> cost) {
        problem.supply_costs[i][parsed] = cost;
        parsed++;
      }
    }
  }

  return problem;
}

std::vector<double> Knapsack(const std::vector<double>& ys) {
  std::vector<double> solution;
  double sum = 0;
  int k;
  for (k = 0; k < ys.size() && sum < 1; k++) {
    sum += ys[k];
  }
  solution.reserve(k);
  for (int i = 0; i < k - 1; i++) {
    solution.push_back(ys[i]);
  }
  solution.push_back(1 - sum + ys[k - 1]);

  return solution;
}
} // namespace math_opt_benchmark
