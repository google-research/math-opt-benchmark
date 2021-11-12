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

// TODO differentiate between triangular and normal matrices

#include <iostream>
#include <vector>

#include "ortools/base/logging.h"

namespace math_opt_benchmark {
template <class T>
class Matrix {
 public:
  Matrix<T>() = default;
  Matrix<T>(int n) { init(n); };

  void init_triangular(int rows);
  void init(int rows, int cols);
  T get(int row, int col) const;
  void set(int row, int col, T value);
  std::pair<int, int> size() const;
  int num_rows() const;
  int num_cols() const;
  void print() const;
  std::vector<std::vector<T>> as_vector_vector() const;
  std::vector<T> as_vector(int row) const;

 private:
  int num_rows_;
  int num_cols_;
  std::vector<std::vector<T>> elements_;
  std::vector<std::vector<bool>> set_;
};

/*
 *
 * TEMPLATE IMPLEMENTATION
 *
 */

template <class T>
void Matrix<T>::init_triangular(int rows) {
  num_rows_ = rows;
  for (int i = 0; i < rows; ++i) {
    elements_.emplace_back(i + 1);
    set_.emplace_back(i + 1, false);
  }
}

template<class T>
void Matrix<T>::init(int rows, int cols) {
  num_rows_ = rows;
  num_cols_ = cols;
  for (int i = 0; i < rows; i++) {
    elements_.emplace_back(cols);
    set_.emplace_back(cols, false);
  }
}

template <class T>
T Matrix<T>::get(int row, int col) const {
  CHECK_GE(row, 0);
  CHECK_GE(col, 0);
  // TODO remove
  int r = row > col ? row : col;
  int c = row > col ? col : row;
  CHECK_LT(r, elements_.size());
  CHECK_LT(c, elements_[r].size());
  CHECK_EQ(set_[r][c], true);
  return elements_[r][c];
}

template <class T>
void Matrix<T>::set(int row, int col, T value) {
  // TODO remove
  int r = row > col ? row : col;
  int c = row > col ? col : row;
  set_[r][c] = true;
  elements_[r][c] = value;
}

template <class T>
std::pair<int, int> Matrix<T>::size() const {
  return std::pair<int, int>(num_rows_, num_cols_);
}

template <class T>
int Matrix<T>::num_rows() const {
  return num_rows_;
}

template <class T>
int Matrix<T>::num_cols() const {
  return num_cols_;
}

template <class T>
void Matrix<T>::print() const {
  for (int i = 0; i < num_rows_; i++) {
    printf("SET: ");
    for (int j = 0; j < i; j++) {
      std::cout << set_[i][j] << ' ';
    }
    printf("\n");
  }
  printf("----------\n");
  for (int i = 0; i < num_rows_; i++) {
    printf("GET: ");
    for (int j = 0; j < i; j++) {
      std::cout << elements_[i][j] << ' ';
    }
    printf("\n");
  }
  printf("----------\n");
}

template <class T>
std::vector<std::vector<T>> Matrix<T>::as_vector_vector() const {
  return elements_;
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

}  // namespace math_opt_benchmark

#endif  // MATH_OPT_BENCHMARK_MATRIX_H
