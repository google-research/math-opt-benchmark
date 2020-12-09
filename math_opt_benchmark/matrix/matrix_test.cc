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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace math_opt_benchmark {
namespace {

TEST(MatrixTest, SortedInsertion) {
  int n = 5;
  Matrix<int> m(n);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j <= i; j++) {
      m.set(i, j, i + j);
      EXPECT_EQ(m.get(i, j), i + j);
    }
  }
}

TEST(MatrixTest, UnsortedInsertion) {
  int n = 5;
  Matrix<int> m(n);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j <= i; j++) {
      m.set(j, i, i + j);
      EXPECT_EQ(m.get(i, j), i + j);
    }
  }
}

TEST(MatrixTest, UnsortedRetrieval) {
  int n = 5;
  Matrix<int> m(n);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j <= i; j++) {
      m.set(i, j, i + j);
      EXPECT_EQ(m.get(j, i), i + j);
    }
  }
}

TEST(MatrixTest, AsVector) {
  int n = 3;
  Matrix<int> m(n);
  std::vector<std::vector<int>> expected;
  // Should I explicitly set the values?
  for (int i = 0; i < n; i++) {
    expected.emplace_back(i + 1);
    for (int j = 0; j <= i; j++) {
      expected[i][j] = i + j;
    }
  }
  for (int i = 0; i < n; i++) {
    for (int j = 0; j <= i; j++) {
      m.set(i, j, i + j);
    }
  }
  EXPECT_EQ(m.as_vector(), expected);
}

TEST(MatrixTest, TemplateTypes) {
  int n = 1;
  Matrix<double> m(n);
  double expected = 1.2345;
  m.set(0, 0, expected);
  double actual = m.get(0, 0);
  EXPECT_DOUBLE_EQ(actual, expected);
}

}  // namespace
}  // namespace math_opt_benchmark
