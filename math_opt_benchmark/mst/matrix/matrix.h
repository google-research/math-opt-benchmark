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

#ifndef MATH_OPT_BENCHMARK_MST_MATRIX_H
#define MATH_OPT_BENCHMARK_MST_MATRIX_H

#include <vector>
#include "glog/logging.h"

namespace math_opt_benchmark {

template <class T>
class Matrix {
public:
  Matrix<T>() {};

  void init(int n) {
    n_ = n;
    for (int i = 0; i < n; ++i) {
      elements_.push_back(std::vector<T>(n));
    }
  }

  T get(int i, int j) const {
    CHECK_GE(i, 0);
    CHECK_GE(j, 0);
    CHECK_LT(i, elements_.size());
    CHECK_LT(j, elements_[i].size());
    return elements_[i][j];
  }

  void set(int i, int j, T value) {
    elements_[i][j] = value;
  }

  int size() const {
    return n_;
  }
private:
  int n_;
  std::vector<std::vector<T>> elements_;
};

}  // namespace math_opt_benchmark

#endif //MATH_OPT_BENCHMARK_MATRIX_H
