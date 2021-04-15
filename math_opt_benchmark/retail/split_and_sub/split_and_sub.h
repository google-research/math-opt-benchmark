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

#ifndef MATH_OPT_BENCHMARK_MATH_OPT_BENCHMARK_FACILITY_RETAIL_H_
#define MATH_OPT_BENCHMARK_MATH_OPT_BENCHMARK_FACILITY_RETAIL_H_

#include "ortools/linear_solver/linear_solver.h"
#include "unordered_map"

namespace math_opt_benchmark {

struct SplitAndSubProblem {
  int capacity;
  std::vector<double> weights;
  std::vector<std::vector<int>> customer_orders;
  std::vector<std::unordered_map<int, std::vector<int>>> substitutions;
};

struct SplitAndSubSolution {
  double objective_value;
  std::vector<double> in_assortment;
  std::vector<double> must_split;
};

class SplitAndSubSolver {
public:
  SplitAndSubSolver(operations_research::MPSolver::OptimizationProblemType problem_type, const SplitAndSubProblem &problem,
                    bool iterative=true);
  SplitAndSubSolution Solve();
  void UpdateObjective(operations_research::MPVariable *var, double value);
  void AddBenderCut(const std::vector<int>& y_indices, const SplitAndSubProblem &problem);

private:
  operations_research::MPSolver solver_;
  std::vector<operations_research::MPVariable *> assortment_vars_;
  std::vector<operations_research::MPVariable *> supply_vars_;
  operations_research::MPVariable *bender_var_;
  bool iterative_;
};

} // namespace math_opt_benchmark

#endif //MATH_OPT_BENCHMARK_MATH_OPT_BENCHMARK_FACILITY_RETAIL_H_