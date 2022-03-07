// Copyright 2022 The MathOpt Benchmark Authors.
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

#include <vector>
#include <sstream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ortools/math_opt/cpp/math_opt.h"

namespace math_opt_benchmark {
namespace {

namespace math_opt = ::operations_research::math_opt;
using ::testing::DoubleNear;
using ::testing::Pointwise;
using ::testing::Eq;
using ::testing::ElementsAreArray;

constexpr double kTolerance = 1e-5;

TEST(ParseTest, SmallInstance) {
  const std::string str("4 3\n"
                           "cap 10\n"
                           "is 20\n"
                           "ignored 300\n"
                           "here 200\n"
                           "demand\n"
                           "0 1 2 3\n"
                           "is\n"
                           "40 50 60 70\n"
                           "ignored\n"
                           "1.1 11.1 111. 1111\n"
                           );
  const UFLProblem problem = ParseProblem(str);
  const int expect_num_customers = 3;
  const int expect_num_facilities = 4;
  const std::vector<double> expect_open_costs = {10, 20, 300, 200};
  const std::vector<std::vector<double>> expect_supply_costs = {{0, 1, 2, 3},
                                                       {40, 50, 60, 70},
                                                       {1.1, 11.1, 111.0, 1111.0}};
  EXPECT_THAT(problem.num_facilities, Eq(expect_num_facilities));
  EXPECT_THAT(problem.num_customers, Eq(expect_num_customers));
  EXPECT_THAT(problem.open_costs, ElementsAreArray(expect_open_costs));
  for (int i = 0; i < problem.num_customers; i++) {
    EXPECT_THAT(problem.supply_costs[i], ElementsAreArray(expect_supply_costs[i]));
  }

}

TEST(KnapsackTest, EasyInstance) {
  const std::vector<double> open_facilities({0.5, 0.4, 0.3, 0.2, 0.1, 0.0});
  const std::vector<double> result = Knapsack(open_facilities);
  const std::vector<double> expect({0.5, 0.4, 0.1});
  EXPECT_THAT(result, Pointwise(DoubleNear(kTolerance), expect));
}

TEST(KnapsackTest, CorrectBounds) {
  const std::vector<double> open_facilities({0.0, 0.0, 0.0});
  const std::vector<double> result = Knapsack(open_facilities);
  const std::vector<double> expect({0.0, 0.0, 1.0});
  EXPECT_THAT(result, Pointwise(DoubleNear(kTolerance), expect));
}

TEST(UFLSolverTest, TwoFacilities) {
  UFLProblem problem;
  problem.num_facilities = 2;
  problem.num_customers = 2;
  problem.open_costs = {1.0, 0.5};
  problem.supply_costs = {{1.0, 0.5},
                          {0.5, 1.0}};
  UFLBenders solver(problem, math_opt::SolverType::kGscip);
  UFLSolution solution = solver.Solve();
  const std::vector<double> expect_open({0.0, 1.0});
  const std::vector<int> expect_supply({1, 1});
  const double expect_obj = 0.5 + 0.5 + 1.0;
  EXPECT_NEAR(solution.objective_value, expect_obj, kTolerance);
  EXPECT_THAT(solution.open_values, Pointwise(DoubleNear(kTolerance), expect_open));
  EXPECT_THAT(solution.supply_values, Pointwise(Eq(), expect_supply));
}

TEST(UFLSolverTest, OnlySupply) {
  UFLProblem problem;
  problem.num_facilities = 2;
  problem.num_customers = 4;
  problem.open_costs = {0.0, 0.0};
  problem.supply_costs = {{1, 2},
                          {2, 1},
                          {2, 3},
                          {3, 4}};
  UFLBenders solver(problem);
  const UFLSolution solution = solver.Solve();
  const std::vector<double> expect_open({1.0, 1.0});
  const std::vector<int> expect_supply({0, 1, 0, 0});
  double expect_obj = 1 + 1 + 2 + 3;
  EXPECT_NEAR(solution.objective_value, expect_obj, kTolerance);
  EXPECT_THAT(solution.open_values, Pointwise(DoubleNear(kTolerance), expect_open));
  EXPECT_THAT(solution.supply_values, Pointwise(Eq(), expect_supply));
}

TEST(UFLSolverTest, OnlyOpen) {
  UFLProblem problem;
  problem.num_facilities = 3;
  problem.num_customers = 1;
  problem.open_costs = {0.5, 0.5, 0.4};
  problem.supply_costs = {{0, 0, 0}};
  UFLBenders solver(problem);
  const UFLSolution solution = solver.Solve();
  const std::vector<double> expect_open({0.0, 0.0, 1.0});
  const double expect_obj = 0.4;
  EXPECT_NEAR(solution.objective_value, expect_obj, kTolerance);
  EXPECT_THAT(solution.open_values, Pointwise(DoubleNear(kTolerance), expect_open));
}

}  // namespace
}  // namespace math_opt_benchmark
