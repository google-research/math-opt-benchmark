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

#include "split_and_sub.h"

#include "absl/strings/str_cat.h"
#include "absl/random/random.h"
#include "absl/strings/str_join.h"
#include "ortools/math_opt/indexed_model.h"

constexpr double kInf = std::numeric_limits<double>::infinity();

template <class T>
void PrintVector(std::vector<T> vec, const std::string& name="Vector") {
  std::cout << name << ": " << absl::StrJoin(vec, ",") << std::endl;
}

namespace math_opt_benchmark {

namespace math_opt = operations_research::math_opt;

using ::operations_research::math_opt::LinearConstraint;
using ::operations_research::math_opt::MathOpt;
using ::operations_research::math_opt::Objective;
using ::operations_research::math_opt::Result;
using ::operations_research::math_opt::SolveParametersProto;
using ::operations_research::math_opt::SolveResultProto;
using ::operations_research::math_opt::Variable;
using ::operations_research::math_opt::IndexedModel;
using ::operations_research::math_opt::VariableId;

/**
 * @param problem_type MPSolver constant specifying which solver to use
 * @param problem Facility location specification with costs and sizes
 */
SplitAndSubSolver::SplitAndSubSolver(operations_research::math_opt::SolverType problem_type, const SplitAndSubProblem &problem,
                                     bool iterative, bool is_integral)
    : optimizer_(problem_type, "Retail_solver"),
    bender_var_(optimizer_.AddContinuousVariable(0.0, kInf, "z")),
    iterative_(iterative) {
  optimizer_.objective().set_minimize();
  UpdateObjective(bender_var_, 1);

  assortment_vars_.reserve(problem.weights.size());
  for (int i = 0; i < problem.weights.size(); i++) {
    const Variable var = optimizer_.AddVariable(0.0, 1.0, is_integral, absl::StrCat("x", i));
    assortment_vars_.push_back(var);
  }

  // Sum of weights less than capacity
  const LinearConstraint weight_limit = optimizer_.AddLinearConstraint(0, problem.capacity);
  for (int i = 0; i < problem.weights.size(); i++) {
    weight_limit.set_coefficient(assortment_vars_[i], problem.weights[i]);
  }

  if (!iterative) {
    supply_vars_.reserve(problem.customer_orders.size());
    for (int i = 0; i < problem.customer_orders.size(); i++) {
      const Variable var = optimizer_.AddContinuousVariable(0.0, 1.0, absl::StrCat("s", i));
      supply_vars_.push_back(var);
      UpdateObjective(supply_vars_[i], 1);
    }

    std::vector<int> item_coefficients(problem.weights.size(), 0);
    for (int i = 0; i < problem.customer_orders.size(); i++) {
      for (int j = 0; j < problem.customer_orders[i].size(); j++) {
        const LinearConstraint split_constraint = optimizer_.AddLinearConstraint(1, kInf);
        split_constraint.set_coefficient(supply_vars_[i], 1);
        int item = problem.customer_orders[i][j];
        if (problem.substitutions[i].count(item)) {
          std::vector<int> subs = problem.substitutions[i].find(item)->second;
          for (int sub : subs) {
            item_coefficients[sub]++;
          }
        }
        item_coefficients[item]++;
        for (int k = 0; k < item_coefficients.size(); k++) {
          split_constraint.set_coefficient(assortment_vars_[k], item_coefficients[k]);
        }
        std::fill(item_coefficients.begin(), item_coefficients.end(), 0);
      }
    }
  }
}

/**
 * Calls the MPSolver solve routine and stores the result in a solution object
 * @return Solution containing objective value and variable values
 */
SplitAndSubSolution SplitAndSubSolver::Solve() {
  math_opt::SolveParametersProto solve_parameters;
  absl::StatusOr<math_opt::Result> result = optimizer_.Solve(solve_parameters);
  CHECK_EQ(result.value().termination_reason, SolveResultProto::OPTIMAL) << result.value().termination_detail;
  SplitAndSubSolution solution;
  solution.solve_time = result->solve_time();
  solution.objective_value = result.value().objective_value();
  solution.in_assortment.reserve(assortment_vars_.size());
  for (auto assortment_var : assortment_vars_) {
    solution.in_assortment.push_back(result.value().variable_values().at(assortment_var));
  }
  if (!iterative_) {
    solution.must_split.reserve(supply_vars_.size());
    for (auto supply_var : supply_vars_) {
      solution.must_split.push_back(result.value().variable_values().at(supply_var));
    }
  }

  return solution;
}

/**
 * Verifies input and updates the coefficient for a variable
 * @param var MPVariable in the solver
 * @param value New variable coefficient
 */
void SplitAndSubSolver::UpdateObjective(const Variable var, double value) {
  optimizer_.objective().set_linear_coefficient(var, value);
}

int sort_by_size(const std::vector<int> &a, const std::vector<int> &b) {
  return a.size() < b.size();
}

/**
 * Adds a Benders cut for each
 * @param sum
 * @param y_coefficients
 */
void SplitAndSubSolver::AddBenderCut(std::vector<int> &y_indices, const SplitAndSubProblem& problem) {
  int num_no_solutions = std::count(y_indices.begin(), y_indices.end(), -1);
  std::replace(y_indices.begin(), y_indices.end(), -1, 0);
  size_t num_solutions = y_indices.size() - num_no_solutions;
  const LinearConstraint cut = optimizer_.AddLinearConstraint(num_solutions, kInf);
  cut.set_coefficient(bender_var_, 1);
  for (int i = 0; i < y_indices.size(); i++) {
    int index = y_indices[i];
    if (index >= 0) {
      CHECK_LT(index, assortment_vars_.size());
      cut.set_coefficient(assortment_vars_[index], cut.coefficient(assortment_vars_[index]) + 1);
      if (problem.substitutions[i].count(index)) {
        for (int sub_index : problem.substitutions[i].find(index)->second) {
          cut.set_coefficient(assortment_vars_[sub_index], cut.coefficient(assortment_vars_[sub_index]) + 1);
        }
      }
    }
  }
}
math_opt::ModelProto SplitAndSubSolver::GetModelProto() {
  return optimizer_.ExportModel();
}

math_opt::ModelUpdateProto SplitAndSubSolver::GetUpdateProto() {
  return optimizer_.ExportModelUpdate();
}

void SplitAndSubSolver::MakeIntegral() {
  for (const math_opt::Variable& x: assortment_vars_) {
    x.set_integer();
  }
}

} // namespace math_opt_benchmark