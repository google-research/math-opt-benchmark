// Copyright 2023 The MathOpt Benchmark Authors.
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

#ifndef MATH_OPT_BENCHMARK_EXAMPLE_EXAMPLE_H_
#define MATH_OPT_BENCHMARK_EXAMPLE_EXAMPLE_H_

#include <memory>
#include <vector>

#include "ortools/math_opt/cpp/math_opt.h"

namespace math_opt_benchmark {

// Models the problem:
//
// max_x sum_i c_i*x_i
// s.t.  sum_i x_i <= d
//             x_i in [0, 1]
//
// where optionally the x_i can be required to be integer.
struct ExampleProblem {
  std::vector<double> objective;
  double rhs = 1.0;
  bool integer = false;
};

struct ExampleSolution {
  double objective_value;
  std::vector<double> x_values;
};

class ExampleSolver {
 public:
  ExampleSolver(operations_research::math_opt::SolverType solver_type,
                const ExampleProblem& problem);
  ExampleSolution Solve();
  void UpdateObjective(int index, double value);

 private:
  operations_research::math_opt::Model model_;
  std::vector<operations_research::math_opt::Variable> x_vars_;
  std::unique_ptr<operations_research::math_opt::IncrementalSolver> solver_;
};

}  // namespace math_opt_benchmark

#endif  // MATH_OPT_BENCHMARK_EXAMPLE_EXAMPLE_H_
