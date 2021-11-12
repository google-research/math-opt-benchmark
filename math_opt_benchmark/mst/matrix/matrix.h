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

#include <iostream>
#include <vector>

#include "ortools/math_opt/cpp/math_opt.h"
#include "absl/strings/str_join.h"

template <class T>
void PrintVector(std::vector<T> vec) {
  std::cout << "Vector: " << absl::StrJoin(vec, ",") << std::endl;
}

namespace math_opt_benchmark {
template <class T>
class Matrix {
 public:
  Matrix<T>() = default;
  Matrix<T>(int n) { init(n); };

  void init(int n, void *default_value=nullptr);
  T get(int i, int j) const;
  void set(int i, int j, T value);
  int size() const;
  void print() const;
  std::vector<std::vector<T>> as_vector_vector() const;
  std::vector<T> as_vector(int row) const;
  std::vector<int> set_vector(int row) const;
  bool is_set(int i, int j) const;

  std::vector<std::vector<T>> elements_;
 private:
  int n_{};
  std::vector<std::vector<bool>> set_;
};

/*
 *
 * TEMPLATE IMPLEMENTATION
 *
 */

template <class T>
void Matrix<T>::init(int n, void *default_value /* = nullptr */) {
  default_value = default_value ? default_value : std::malloc(sizeof(T));
  n_ = n;
  for (int i = 0; i < n; ++i) {
    elements_.emplace_back(i + 1, *((T *) default_value));
    CHECK_EQ(i+1, elements_[i].size());
    set_.emplace_back(i + 1, false);
  }
}

template <class T>
T Matrix<T>::get(int i, int j) const {
  CHECK_GE(i, 0);
  CHECK_GE(j, 0);
  int r = i > j ? i : j;
  int c = i > j ? j : i;
  CHECK_LT(r, elements_.size());
  CHECK_LE(c, elements_[r].size());
  CHECK_EQ(set_[r][c], true);
  return elements_[r][c];
}

template <class T>
void Matrix<T>::set(int i, int j, T value) {
  int r = i > j ? i : j;
  int c = i > j ? j : i;
  set_[r][c] = true;
  elements_[r][c] = value;
}

template <class T>
int Matrix<T>::size() const {
  return n_;
}

template <class T>
void Matrix<T>::print() const {
  for (int i = 0; i < n_; i++) {
    printf("SET: ");
    for (int j = 0; j <= i; j++) {
      std::cout << set_[i][j] << ' ';
    }
    printf("\n");
  }
  printf("----------\n");
  for (int i = 0; i < n_; i++) {
    printf("GET: (%-2d) ", i);
    for (int j = 0; j <= i; j++) {
      std::cout << elements_[i][j] << ' ';
    }
    printf("\n");
  }
  printf("----------\n");
}

template <class T>
std::vector<std::vector<T>> Matrix<T>::as_vector_vector() const {
  std::vector<std::vector<T>> elements(n_, std::vector(n_, T{}));
  for (int i = 0; i < n_; i++) {
    for (int j = 0; j <= i; i++) {
      if (set_[i][j]) {
        elements[i][j] = elements[j][i] = elements_[i][j];
      }
    }
  }
  return elements;
}

template <class T>
std::vector<T> Matrix<T>::as_vector(int row) const {
  std::vector<T> v;
  for (int i = 0; i < elements_[row].size(); i++) {
    if (set_[row][i]) {
      v.push_back(elements_[row][i]);
    }
  }
  return v;
}

template <class T>
std::vector<int> Matrix<T>::set_vector(int row) const {
  std::vector<int> indices;
  for (int i = 0; i <= row; i++) {
    if (set_[row][i]) {
      indices.push_back(i);
    }
  }
  for (int j = row + 1; j < n_; j++) {
    if (set_[j][row]) {
      indices.push_back(j);
    }
  }
  return indices;
}

template <class T>
bool Matrix<T>::is_set(int i, int j) const {
  int r = i > j ? i : j;
  int c = i > j ? j : i;
  return set_[r][c];
}

}  // namespace math_opt_benchmark

#endif  // MATH_OPT_BENCHMARK_MATRIX_H
