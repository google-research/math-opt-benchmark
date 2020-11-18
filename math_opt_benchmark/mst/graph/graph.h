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

#ifndef MATH_OPT_BENCHMARK_MST_GRAPH_H
#define MATH_OPT_BENCHMARK_MST_GRAPH_H

#include <vector>

#include "math_opt_benchmark/mst/matrix/matrix.h"

namespace math_opt_benchmark {

class Graph {
 public:
  // Should there be two const&'s?
  Graph(std::vector<std::vector<int>>&& edges);
  std::vector<std::vector<int>> invalid_components(
      const Matrix<double>& x_values);

 private:
  int n_;
  std::vector<std::vector<int>> edges_;
};

}  // namespace math_opt_benchmark

#endif  // MATH_OPT_BENCHMARK_MST_GRAPH_H
