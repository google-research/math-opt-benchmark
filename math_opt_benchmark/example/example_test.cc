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

#include "math_opt_benchmark/example/example.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ortools/linear_solver/linear_solver.h"

namespace math_opt_benchmark {
namespace {

constexpr auto kGlop = ::operations_research::MPSolver::GLOP_LINEAR_PROGRAMMING;
constexpr auto kScip =
    ::operations_research::MPSolver::SCIP_MIXED_INTEGER_PROGRAMMING;
constexpr double kTolerance = 1e-5;

using ::testing::DoubleNear;
using ::testing::Pointwise;

TEST(ExampleSolverTest, OneVariable) {
  ExampleProblem problem;
  problem.rhs = 1.0;
  problem.objective = {3.5};
  problem.integer = false;
  ExampleSolver solver(kGlop, problem);
  const ExampleSolution solution = solver.Solve();
  EXPECT_NEAR(solution.objective_value, 3.5, kTolerance);
  const std::vector<double> expected_var_values = {1.0};
  EXPECT_THAT(solution.x_values,
              Pointwise(DoubleNear(kTolerance), expected_var_values));
}

TEST(ExampleSolverTest, TwoOfThreeInteger) {
  ExampleProblem problem;
  problem.rhs = 2.1;
  problem.objective = {4.0, 2.0, 6.0};
  problem.integer = true;
  ExampleSolver solver(kScip, problem);
  const ExampleSolution solution = solver.Solve();
  EXPECT_NEAR(solution.objective_value, 10.0, kTolerance);
  const std::vector<double> expected_var_values = {1.0, 0.0, 1.0};
  EXPECT_THAT(solution.x_values,
              Pointwise(DoubleNear(kTolerance), expected_var_values));
}

TEST(ExampleSolverTest, FractionalRhsContinuousVariables) {
  ExampleProblem problem;
  problem.rhs = 2.5;
  problem.objective = {4.0, 2.0, 6.0};
  problem.integer = false;
  ExampleSolver solver(kGlop, problem);
  const ExampleSolution solution = solver.Solve();
  EXPECT_NEAR(solution.objective_value, 11.0, kTolerance);
  const std::vector<double> expected_var_values = {1.0, 0.5, 1.0};
  EXPECT_THAT(solution.x_values,
              Pointwise(DoubleNear(kTolerance), expected_var_values));
}

TEST(ExampleSolverTest, Update) {
  ExampleProblem problem;
  problem.rhs = 2.0;
  problem.objective = {4.0, 2.0, 6.0};
  problem.integer = false;
  ExampleSolver solver(kGlop, problem);
  EXPECT_NEAR(10.0, solver.Solve().objective_value, kTolerance);
  solver.UpdateObjective(1, 5.0);
  const ExampleSolution solution = solver.Solve();
  EXPECT_NEAR(solution.objective_value, 11.0, kTolerance);
  const std::vector<double> expected_var_values = {0.0, 1.0, 1.0};
  EXPECT_THAT(solution.x_values,
              Pointwise(DoubleNear(kTolerance), expected_var_values));
}

}  // namespace
}  // namespace math_opt_benchmark
