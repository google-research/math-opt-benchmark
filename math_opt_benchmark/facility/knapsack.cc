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

#include "math_opt_benchmark/matrix/matrix.h"

#include <vector>
#include <cmath>

constexpr double kTolerance = 1e-5;

namespace math_opt_benchmark {

// Assume costs are column-wise increasing and ys accordingly
std::vector<std::vector<double>> knapsack(const Matrix<double>& costs, const std::vector<double>& ys) {
  std::vector<std::vector<double>> solution(costs.size());
  // For every customer
  for (int i = 0; i < costs.size(); i++) {
    double sum = 0;
    int k;
    for (k = 0; k < ys.size() && sum < 1; k++) {
      sum += ys[k];
    }
    for (int j = 0; j < k-1; j++) {
      solution[i].push_back(ys[j]);
    }
    solution[i].push_back(1 - sum - ys[k]);
  }

  return solution;
}

}  // namespace math_opt_benchmark