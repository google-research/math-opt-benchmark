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

#include "absl/strings/str_cat.h"

constexpr double kInf = std::numeric_limits<double>::infinity();

namespace math_opt_benchmark {


/**
 * @param problem_type MPSolver constant specifying which solver to use
 * @param problem Graph specification with edges, weights, and number of vertices
 */
MSTSolver::MSTSolver(const math_opt::SolverType problem_type,
                     const MSTProblem &problem)
    : optimizer_(problem_type, "MST_solver") {
  optimizer_.objective().set_minimize();
  x_vars_.init(problem.n);
  math_opt::LinearConstraint c_eq = optimizer_.AddLinearConstraint(problem.n - 1, problem.n - 1);
  for (int i = 0; i < problem.n; i++) {
    for (int j = 0; j < i; j++) {
      if (problem.edges.is_set(i, j) && problem.edges.get(i, j)) {
        math_opt::Variable var = optimizer_.AddVariable(0.0, 1.0, problem.integer, absl::StrCat("x", i, ",", j));
        x_vars_.set(i, j, var);
        UpdateObjective(i, j, problem.weights.get(i, j));
        c_eq.set_coefficient(var, 1);
      }
    }
  }

  *(model_.mutable_initial_model()) = optimizer_.ExportModel();
}

void debug_solve(const MSTSolution &result) {
  for (int i = 0; i < result.x_values.size(); i++) {
    for (int j = 0; j < result.x_values.size(); j++) {
      if (result.x_values.is_set(i, j)) {
        printf("[D] x[%i][%i] = %.7f\n", i, j, result.x_values.get(i, j));
      }
    }
  }
}

/**
 * Calls the MPSolver solve routine and stores the result in a solution object
 * @return Solution containing objective value and variable values
 */
MSTSolution MSTSolver::Solve() {
  math_opt::SolveParametersProto solve_parameters;
  solve_parameters.mutable_common_parameters()->set_enable_output(false);
  absl::StatusOr<math_opt::Result> result = optimizer_.Solve(solve_parameters);
  CHECK_EQ(result.value().termination_reason, math_opt::SolveResultProto::OPTIMAL) << result.value().termination_detail;

  MSTSolution solution;
  solution.objective_value = result.value().objective_value();
  solution.x_values.init(x_vars_.size());
  for (int i = 0; i < x_vars_.size(); i++) {
    for (int j = 0; j < i; j++) {
      if (x_vars_.is_set(i, j)) {
        solution.x_values.set(i, j, result.value().variable_values().at(x_vars_.get(i, j)));
      }
    }
  }

  model_.mutable_objectives()->Add(solution.objective_value);

  return solution;
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
  CHECK_LT(v2, x_vars_.size());
  optimizer_.objective().set_linear_coefficient(x_vars_.get(v1, v2), value);
}

int sort_by_size(const std::vector<int> &a, const std::vector<int> &b) {
  return a.size() < b.size();
}

/**
 * Adds a subtour-elimination constraint for each list of constraint-violating vertices
 * @param problem Graph specification with edges, weights, and number of vertices
 * @param invalid List of lists of vertices that violate the MST constraint
 */
void MSTSolver::AddConstraints(const MSTProblem &problem,
                               std::vector<std::vector<int>> invalid) {
  constexpr int kMaxConstraintVars = 100;
  std::sort(invalid.begin(), invalid.end(), sort_by_size);
  for (int i = 0; i < invalid.size() && i < kMaxConstraintVars; ++i) {
    math_opt::LinearConstraint c = optimizer_.AddLinearConstraint(-kInf, ((double) invalid[i].size()) - 1);
    for (int j = 0; j < invalid[i].size(); j++) {
      for (int k = 0; k < invalid[i].size(); k++) {
        int v1 = invalid[i][j], v2 = invalid[i][k];
        if (x_vars_.is_set(v1, v2)) {
          c.set_coefficient(x_vars_.get(v1, v2), 1);
        }
      }
    }
    *(model_.add_model_updates()) = optimizer_.ExportModelUpdate();
  }
}

void MSTSolver::EnforceInteger() {
  for (int i = 0; i < x_vars_.size(); i++) {
    for (int j = 0; j < i; j++) {
      if (x_vars_.is_set(i, j)) {
        x_vars_.get(i, j).set_is_integer(true);
      }
    }
  }
}

BenchmarkInstance MSTSolver::GetModel() {
  return model_;
}

}  // namespace math_opt_benchmark
