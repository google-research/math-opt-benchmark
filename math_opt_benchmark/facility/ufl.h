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

#ifndef MATH_OPT_BENCHMARK_MATH_OPT_BENCHMARK_FACILITY_UFL_H_
#define MATH_OPT_BENCHMARK_MATH_OPT_BENCHMARK_FACILITY_UFL_H_

#include "ortools/math_opt/cpp/math_opt.h"
#include "math_opt_benchmark/proto/model.pb.h"

namespace math_opt = operations_research::math_opt;

namespace math_opt_benchmark {

struct UFLProblem {
  int num_facilities;
  int num_customers;
  std::vector<double> open_costs;
  std::vector<std::vector<double>> supply_costs;
};

struct UFLSolution {
  double objective_value;
  std::vector<double> open_values;
  std::vector<int> supply_values;
};

class UFLSolver {
public:
  UFLSolver(math_opt::SolverType problem_type,
            const UFLProblem &problem, bool iterative);
  UFLSolution Solve();
  void UpdateObjective(math_opt::Variable var, double value);
  void AddBenderCut(double sum, const std::vector<double> &y_coefficients);
  void EnforceInteger();
  BenchmarkInstance GetModel();

private:
  math_opt::MathOpt optimizer_;
  std::vector<std::vector<math_opt::Variable>> supply_vars_;
  std::vector<math_opt::Variable> open_vars_;
  math_opt::Variable bender_var_;
  BenchmarkInstance model_;
};

} // namespace math_opt_benchmark

#endif //MATH_OPT_BENCHMARK_MATH_OPT_BENCHMARK_FACILITY_UFL_H_
