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

#include "math_opt_benchmark/matrix/matrix.h"
#include "ortools/linear_solver/linear_solver.h"

namespace math_opt_benchmark {

struct UFLProblem {
  bool integer;
  int num_facilities;
  int num_customers;
  std::vector<double> open_costs;
  Matrix<double> supply_costs;
};

struct UFLSolution {
  Matrix<double> supply_values;
  std::vector<double> open_values;
  double objective_value;
};

class UFLSolver {
public:
  UFLSolver(operations_research::MPSolver::OptimizationProblemType problem_type,
            const UFLProblem &problem);
  UFLSolution Solve();
  void UpdateObjective(int facility, int customer, double value);
  void UpdateObjective(int facility, double value);
  void AddConstraints(const UFLProblem &problem, std::vector <std::vector<int>> invalid);

private:
  operations_research::MPSolver solver_;
  Matrix<operations_research::MPVariable *> supply_vars_;
  std::vector<operations_research::MPVariable *> open_vars_;
};

} // namespace math_opt_benchmark

#endif //MATH_OPT_BENCHMARK_MATH_OPT_BENCHMARK_FACILITY_UFL_H_
