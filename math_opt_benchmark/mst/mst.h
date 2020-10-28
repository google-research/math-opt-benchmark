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

#ifndef MATH_OPT_BENCHMARK_MST_H
#define MATH_OPT_BENCHMARK_MST_H

#include <vector>

#include "ortools/linear_solver/linear_solver.h"
#include "math_opt_benchmark/mst/matrix/matrix.h"

// DEFINE_double(EPSILON, 0.0001, "Accuracy when comparing solver values");

namespace math_opt_benchmark {

struct MSTProblem {
  int n;
  Matrix<double> weights;
  Matrix<int> edges;
  bool integer = false;
};

struct MSTSolution {
  double objective_value;
  Matrix<double> x_values;
};

class MSTSolver {
 public:
  MSTSolver(operations_research::MPSolver::OptimizationProblemType problem_type,
            const MSTProblem& problem);
  MSTSolution Solve();
  void UpdateObjective(int v1, int v2, double value);
  void AddConstraints(const MSTProblem& problem,
                      std::vector<std::vector<int>> invalid);

 private:
  operations_research::MPSolver solver_;
  Matrix<operations_research::MPVariable*> x_vars_;
};

}  // namespace math_opt_benchmark

#endif  // MATH_OPT_BENCHMARK_MST_H
