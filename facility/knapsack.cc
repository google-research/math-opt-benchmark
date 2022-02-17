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

#include "math_opt_benchmark/facility/knapsack.h"

namespace math_opt_benchmark {

// Assume costs and ys are sorted together so costs[i] < costs[i+1]
std::vector<double> knapsack(const std::vector<double>& costs, const std::vector<double>& ys) {
  std::vector<double> solution;
  double sum = 0;
  int k;
  for (k = 0; k < ys.size() && sum < 1; k++) {
    sum += ys[k];
    solution.push_back(ys[k]);
  }
  solution.push_back(1 - sum - ys[k-1]);

  return solution;
}

}  // namespace math_opt_benchmark