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

#include "ortools/math_opt/cpp/math_opt.h"
#include "math_opt_benchmark/proto/model.pb.h"

#include "unordered_map"

namespace math_opt_benchmark {

struct SplitAndSubProblem {
  int capacity;
  std::vector<double> weights;
  std::vector<std::vector<int>> customer_orders;
  std::vector<std::unordered_map<int, std::vector<int>>> substitutions;
};

struct SplitAndSubSolution {
  absl::Duration solve_time;
  double objective_value;
  std::vector<double> in_assortment;
  std::vector<int> must_split;
};

class SplitAndSubSolver {
public:
  SplitAndSubSolver(operations_research::math_opt::SolverType problem_type, const SplitAndSubProblem &problem,
                    bool iterative, bool is_integral);
  SplitAndSubSolution Solve();
  void UpdateObjective(operations_research::math_opt::Variable var, double value);
  void AddBenderCut(std::vector<int> &y_indices, const SplitAndSubProblem& problem);
  operations_research::math_opt::ModelProto GetModelProto();
  operations_research::math_opt::ModelUpdateProto GetUpdateProto();
  void MakeIntegral();

private:
  operations_research::math_opt::MathOpt optimizer_;
  std::vector<operations_research::math_opt::Variable> assortment_vars_;
  std::vector<operations_research::math_opt::Variable> supply_vars_;
  operations_research::math_opt::Variable bender_var_;
  bool iterative_;
};

} // namespace math_opt_benchmark

#endif //MATH_OPT_BENCHMARK_MATH_OPT_BENCHMARK_FACILITY_RETAIL_H_