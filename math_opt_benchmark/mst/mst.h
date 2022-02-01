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

#ifndef MATH_OPT_BENCHMARK_MST_MST_H
#define MATH_OPT_BENCHMARK_MST_MST_H

#include <vector>

#include "math_opt_benchmark/mst/matrix/matrix.h"
#include "ortools/math_opt/cpp/math_opt.h"
#include "math_opt_benchmark/proto/model.pb.h"

namespace math_opt = operations_research::math_opt;

namespace math_opt_benchmark {

/**
 * n Number of vertices
 * weights Edge-weights indexed by vertex pairs
 * edges List of adjacent vertices for each vertex
 */
struct MSTProblem {
  int n;
  Matrix<double> weights;
  Matrix<int> edges;
  bool integer = false;
};

/**
 * objective_value Objective value returned from the LP solve
 * x_values Solution values corresponding to variable x_vars_[i][j] in a
 * MSTSolver
 */
struct MSTSolution {
  double objective_value;
  Matrix<double> x_values;
};

class MSTSolver {
public:
  MSTSolver(math_opt::SolverType problem_type,
            const MSTProblem& problem);
  MSTSolution Solve();
  void UpdateObjective(int v1, int v2, double value);
  void AddConstraints(const MSTProblem& problem, std::vector<std::vector<int>> invalid);
  void EnforceInteger();
  BenchmarkInstance GetModel();

private:
  math_opt::Model model_;
  std::unique_ptr<math_opt::IncrementalSolver> solver_;
  std::unique_ptr<math_opt::UpdateTracker> update_tracker_;
  Matrix<math_opt::Variable> x_vars_;
  BenchmarkInstance instance_;
};

}  // namespace math_opt_benchmark

#endif  // MATH_OPT_BENCHMARK_MST_MST_H
